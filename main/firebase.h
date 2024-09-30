#ifndef FIREBASE_H
#define FIREBASE_H
#define RELAY_GPIO_PIN 22
void firebase_init(void);
void check_schedule_and_execute(void);
void get_schedule_from_firebase(void);
#endif // FIREBASE_H
