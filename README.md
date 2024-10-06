
# Litteruino Box

Automatic Cat Litter Box Cleaner Code in Arduino




## How to use Litteruino Box:

#### Set Time of Cleaning Delay at Setup;

Rotate the Potentiometer Clockwise to Decrease the Time Interval of the Cleaning Process, Counterclockwise to increase.

#### Set Time of Cleaning Delay during the Cleaning Process;

Hold the button for around 6 seconds and wait until a cycle has been finished to enter the Set Time Mode.

#### Entering the Cleaning Cycle/Process;

Press the button 3 times slowly after setting the time.




## Components Used in the Project

To fully understand the project itself, let us know more about the materials used:

`Tower Pro MG995 Digital High Speed Servo Motor` - `x4`

`Micro Servo 9G SG90` - `x1`

`Arduino UNO` - `x1`

`LCD Screen (16x2) with I2C` - `x1`

`Potentiometer` - `x1`

`Button` - `x1`


## Purpose of the Components

#### Get all items


| Item | Description     |
| :-------- | :------- |
| `Servo Motor`      | `Movement of the Scooper, for moving the system forward and backward, moving the scooper up and down.` |
| `Mini Servo` | `For closing and opening the lid` |
| `Button` | `For starting the cleaning cycle, entering set time mode` |
| `Potentiometer` | `For adjusting the time interval values of the cleaning process.` |
| `LCD Screen` | `Monitoring the current process of the cycle, viewing time interval cooldown` |



## Modifying the Code

**Changing the Long Press needed to enter the Time Set Mode during a cleaning cycle:**

```c++
const unsigned long longPressDuration = TIME_INTERVAL_DESIRED_INT_ONLY;
```
Change ```TIME_INTERVAL_DESIRED_INT_ONLY``` to your desired time, must be an integer (Number Values) only.

*Note: The Changed values may not fully replicate the modified time, give an allowance of 3-5 seconds in the actual demonstration.*



**Changing the Values of the Potentiometer for the Cleaning Interval Values:**

```c++
int timeDelayMapped = map(analogRead(A0), 0, 1023, MINIMUM_TIME, MAXIMUM_TIME);
```
Change ```MINIMUM_TIME``` and ```MAXIMUM_TIME``` to your desired time, must be an integer (Number Values) only.



