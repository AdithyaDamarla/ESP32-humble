#include <micro_ros_arduino.h>
#include <ESP32Servo.h>
#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int8.h>


Servo myservo;  // create servo object to control a servo
int servoPin = 12;

#define BATTERY_PIN 36


rcl_subscription_t LEDs_subscriber;
std_msgs__msg__Int8 LEDs_msg;

rcl_subscription_t servo_subscriber;
std_msgs__msg__Int8 servo_msg;

rcl_publisher_t battery_publisher;
std_msgs__msg__Int8 battery_msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define LEFT_LED_PIN 17
#define RIGHT_LED_PIN 16

#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

int8_t get_battery_percentage();
int map_to_percentage(int raw_value);

void error_loop(){
  while(1){
    delay(100);
  }
}

int limitToMaxValue(int value, int maxLimit) {
  if (value > maxLimit) {
    return maxLimit;
  } else {
    return value;
  }
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    int8_t battery_percentage = get_battery_percentage();
    battery_msg.data = battery_percentage;
    RCSOFTCHECK(rcl_publish(&battery_publisher, &battery_msg, NULL));
  }
}

int8_t get_battery_percentage() {
  // Read the voltage from the BATTERY_PIN
  int raw_value = analogRead(BATTERY_PIN);

  // Convert the raw value to battery percentage (0 to 100)
  int battery_percentage = map_to_percentage(raw_value);

  return static_cast<int8_t>(battery_percentage);
}

int map_to_percentage(int raw_value) {
  // Assuming the raw_value represents the battery voltage in the range of 0 to 4095
  // Adjust the following values based on your battery voltage range and voltage divider setup (if any).
  int min_voltage = 2400;    // Minimum voltage reading (corresponding to 0% battery)
  int max_voltage = 3720; // Maximum voltage reading (corresponding to 100% battery)

  // Map the raw value to the battery percentage
  int battery_percentage = map(raw_value, min_voltage, max_voltage, 0, 100);
  return battery_percentage;
}


void LEDs_subscription_callback(const void * msgin)
{  
  const std_msgs__msg__Int8 * msg = (const std_msgs__msg__Int8 *)msgin;

  int8_t value = msg->data;

  switch (value) {
    case 0:
      digitalWrite(LEFT_LED_PIN, LOW);
      digitalWrite(RIGHT_LED_PIN, LOW);
      break;
    case 1:
      digitalWrite(LEFT_LED_PIN, HIGH);
      digitalWrite(RIGHT_LED_PIN, LOW);
      break;
    case 2:
      digitalWrite(LEFT_LED_PIN, LOW);
      digitalWrite(RIGHT_LED_PIN, HIGH);
      break;
    case 3:
      digitalWrite(LEFT_LED_PIN, HIGH);
      digitalWrite(RIGHT_LED_PIN, HIGH);
      break;
    default:
      break;
  }
  
  //digitalWrite(LED_PIN, (msg->data == 0) ? LOW : HIGH);  
}

void servo_callback(const void* msgin) {
  const std_msgs__msg__Int8* msg = (const std_msgs__msg__Int8*)msgin;
  int8_t angle = msg->data;
  int servo_position;
  servo_position = limitToMaxValue(angle, 40);
  myservo.write(servo_position);
}

void setup() {
  //set_microros_transports();
  set_microros_wifi_transports("Pixel_5234", "deyz1234", "192.168.24.143", 8888);

  pinMode(LEFT_LED_PIN, OUTPUT);
  digitalWrite(LEFT_LED_PIN, HIGH);  

  pinMode(RIGHT_LED_PIN, OUTPUT);
  digitalWrite(RIGHT_LED_PIN, HIGH);  
  pinMode(BATTERY_PIN, INPUT);

  delay(2000);

  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);    // standard 50 hz servo
  myservo.attach(servoPin, 1000, 2000); // attaches the servo on pin 18 to the servo object

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "lineturtle_esp32", "", &support));

  // LED subscriber
  RCCHECK(rclc_subscription_init_default(
    &LEDs_subscriber,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int8),
    "LEDs"));
    
//servo subscriber
  RCCHECK(rclc_subscription_init_default(
      &servo_subscriber,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int8),
      "/servo"));

  RCCHECK(rclc_publisher_init_default(
    &battery_publisher,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int8),
    "battery"));

  // create timer,
  const unsigned int timer_timeout = 100;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));
    
  // create executor
  RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator));
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  RCCHECK(rclc_executor_add_subscription(&executor, &LEDs_subscriber, &LEDs_msg, &LEDs_subscription_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_subscription(&executor, &servo_subscriber, &servo_msg, &servo_callback, ON_NEW_DATA));

}

void loop() {
  delay(100);
  RCCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
}
