#pragma once

#include "util/atomic.h"

/**
 * This class handles events from a rotary encoder with a combined
 * button, assuming external pullups are present on all pins.
 *
 * The encoder used has both outputs high when idle (at a detent), and
 * switches through a full cycle (both outputs go low and high again)
 * for each detent moved.
 *
 * When moving clock-wise, output A goes low before output B, and when
 * moving counter-clockwise, output B goes low before output A (and they
 * go high again in the same order).
 *
 * The moment you feel the knob "click" into the next detent matches the
 * moment where both outputs are high again. This is the moment where
 * events should be triggered.
 *
 *   Clockwise:            Counterclockwise:
 *   __.   ._____          ____.   ._____
 * A   |___|             A     |___|
 *   ____.   .___          __.   .___
 * B     |___|           B   |___|
 *           ^                     ^
 *           Event here            Event here
 *
 * To accurately catch these events, this class registers interrupt
 * handlers on both pins. It seems sufficient to only trigger on rising
 * edges to catch the above events, but then spurious events will be
 * triggered when the encoder is turned slightly past a detent and back
 * again (the forward movement will be missed, while the movement back
 * will trigger a spurious event).
 *
 * Hence, this class monitors both edges on both pins, and keeps track
 * of the exact value of the encoder (where each edge adds or subtracts
 * one to the value kept). Whenever the value is divisible by 4, the
 * encoder is at a detent.
 *
 * The process task is run whenever this happens, and fires an
 * appropriate event. It tracks what the last value was that fired an
 * event, so that when encoder changes away from a detent and back again
 * (without touching the next detent), it does not trigger the spurious
 * event described above.
 *
 * For tracking the encoder value, a lookup table approach is used. This
 * is based on the approach described (among other places) here:
 * https://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
 *
 * This uses a lookup table that maps a combination of the previous pin
 * states and the current pin states to the change in value that this
 * should result in. However, the lookup table used here is slightly
 * modified from the one commonly used.
 *
 * The original table contains 1 to indicate clockwise motion, -1 to
 * indicate counter-clockwise motion, 0 to indicate no motion (pin
 * values unchanged ) and also 0 to indicate an invalid transition (e.g.
 * both pins changed, which should never happen in quadrature encoding).
 *
 * The table used here, uses 2 and -2 for the invalid transitions
 * instead of 0. Even though these should never occur, it is not
 * unthinkable that bouncing or noise will very rarely cause them to
 * happen anyway. With the original table, these transitions did not
 * modify the value, but did modify the "previous pin state" that was
 * stored. Effectively, this could cause the tracked value to get an
 * off-by-two error relative to the actual position. Effectively, these
 * invalid transitions mean the encoder position has changed by 2
 * instead of 1, though there is no way to tell which way the encoder
 * has turned.
 *
 * For this reason, the lookup table contains 2 (and -2, to allow any
 * errors to possibly even out) for these invalid transitions. This will
 * keep the value accurate in some cases, but since we're just making a
 * guess as to the direction, it will by off-by-four in other cases.
 * Even though not perfectly accurate, now at least the assumption that
 * the tracked value is a multiple of four will remain valid, regardless
 * of what values are read.
 *
 * This approach means that the reading is pretty resilient against
 * invalid readings. When there is an invalid reading on a single pin,
 * the value might be temporary off-by-one, but this will be corrected
 * when the reading is correct again. Since normally, only one pin
 * should change at a time, this means that any bouncing of the
 * encoder's contacts should be handled without any problems by this
 * code.
 *
 * When both pins return an invalid reading, the absolute position might
 * get off-by-four errors, but the position within a single cycle should
 * remain correct, meaning detents are still detected correctly.
 */
template <uint8_t button_pin, uint8_t pina, uint8_t pinb>
class ButtonEncoder {
  public:
		ButtonEncoder{
      // These have external pullups
      pinMode(button_pin, INPUT_PULLUP /* TEMPORARY with PULLUP! */);
      pinMode(pina, INPUT);
      pinMode(pinb, INPUT);
			
			attachInterrupt(digitalPinToInterrupt(button_pin), button_isr, FALLING);
      attachInterrupt(digitalPinToInterrupt(pina), encoder_isr, CHANGE);
      attachInterrupt(digitalPinToInterrupt(pinb), encoder_isr, CHANGE);
      // Reset any queued events. attachInterrupt does not clear any
      // previously pending interrupts (e.g. due to startup noise, or
      // due to different interrupt settings), so these might trigger
      // right away. Clear away the result of that. See also
      // https://github.com/arduino/Arduino/issues/510
      button_presses = 0;
      encoder_value = encoder_last_processed = 0;
		}
		
		static uint16_t process() {
			uint16_t returnValue = 0;
			int8_t useful_position = 0;
			// Process encoder events
      while (true) {
        // No need to disable interrupts here, since value is a single
        // byte and can be loaded atomically
        int8_t diff = (int8_t)(encoder_value - encoder_last_processed);

        // Generate an event whenever the encoder has moved a full
        // detent (value changed by 4).
        if (diff < -3) {
          encoder_last_processed -= 4;
          useful_position--;
        } else if (diff > 3) {
          encoder_last_processed += 4;
          useful_position++;
        } else {
          // Not turned a full detent, we're done
          break;
        }
      }
			returnValue = useful_position << 8;
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
				if (button_presses < 128)
					returnValue = button_presses & 0x7F;
				else
					returnValue = 0x7F;
				button_presses = 0;
			}
			return returnValue;
		}
		
private:

    static void button_isr() {
      button_presses++;
    }

    static void encoder_isr() {
      // Initialize to 3, assuming the encoder starts at a detent. This
      // matches the initial value of `value`, so even if it is
      // incorrect, it should resynchronise on the first interrupt.
      static uint8_t prev_reading = 3;

      // This lookup table maps clockwise rotation to positive changes.
      static const int8_t table[] PROGMEM = { 0, -1, 1, 2, 1, 0, 2, -1, -1, -2, 0, 1, -2, 1, -1, 0 };
      uint8_t reading = digitalRead(pina) << 1 | digitalRead(pinb);
      uint8_t key = prev_reading << 2 | reading;
      encoder_value += pgm_read_byte(&table[key]);
      prev_reading = reading;

      // When we touch a detent, signal the main loop to process an event
      // TODO: This might cause race conditions, since TaskScheduler
      // does not take care of atomicity
      if (encoder_value % 4 == 0)
        process_task.restart();
    }

    static volatile uint8_t button_presses;
    static uint8_t encoder_last_processed;
    static volatile uint8_t encoder_value;
};
