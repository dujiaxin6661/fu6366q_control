FU6366Q no-load maximum speed test demo

Purpose:
Use this firmware only for no-load maximum-speed identification.
It helps distinguish whether the motor is closer to GB3506 or GB3510 by observing the highest actual speed at 12 V.

Key behavior:
- Power on: wait for MR calibration and mcRun, then stop and wait for a key.
- First GP45 press: start +300 rpm.
- Later GP45 press: target speed +100 rpm per press.
- First GP33 press: start -300 rpm.
- Later GP33 press: target speed -100 rpm per press.
- Speed clamp: -1800 rpm to +1800 rpm.
- Motor keeps running after the key is released.

Suggested test:
1. Run the PC plotter as usual.
2. No load on the output shaft.
3. Press GP45 step by step.
4. Hold each step for 3-5 seconds.
5. Stop increasing when actual speed no longer rises, speed error grows sharply, Vbus drops, or any fault appears.
6. Repeat in reverse direction with GP33 only if needed.

Important:
Do not use this firmware for loaded tests.
Use the normal no-load baseline / load-test firmware for -200..+200 rpm torque evaluation.
