# Project Proposal

## 1. Motivation & Objective

A flat surface can be more than just a surface. You can project onto it into a display. In this project we want to look at using everyday devices to turn a surface into touch sensor. More specifically, acoustic devices placed at the vertices pick up a touch event propagated through air and log the time of arrival. Using the time of arrival, we can solve the location of the touch event. This project involves two challenges, one is to solve the location of the touch location given time of arrival, the other is to make devices sync with each other to get accurate time of arrival. 

## 2. State of the Art & Its Limitations

For implementing touch interaction on large surface, several ways are avaible and some of them are already commercially viable. One way is to use a camera to track the touch event. However, it requires a camera to be placed above the surface and the camera needs to be calibrated. Also privacy can be harmed. Another way is to use a capacitive touch sensor. This is no different from any small touchscreen. But in the case of large surface, the cost and power consumption of the sensor can be high. Also, the sensor needs to be manufactured to adapt to the size. It is not plug and play. Another way is to use infrared pairs. By looking at which part has been blocked, we can infer the touch event. However, this method requires bulky hardware.

The idea of using acoustic to infer touch event is also popular and leads to some innovative work. In Acustico, the time difference between surface wave and sound wave are used to infer touch location. All equipments are on users's wrist. Since the time differnce is small, high sampling rate is necessary. Also, it provides relative location to hand instead of absolute location on the surface. 

In Toffee, microphoes at different edges of the same device are used and the TDoA is used to inference the relative angle of the touch event. It opens new way of user interaction. However, it does not provide localization on the surface.

In ALTo, the time difference of arrival of the event's acoustic signal at a bunch of microphones are used to infer touch location. The microphones are placed at the vertices of the surface. The microphones are connect to the same device for data collection and processing. We recognize this as very stringent as it does not scale to large surface. Also it prevent the possibility of using a bunch of independent devices to infer touch location. We consider this work as an important preliminary work and we want to extend it to a more practical case.

## 3. Novelty & Rationale

We want to use a bunch of independent devices to infer touch location on a large surface. We want to design a protocol between devices to allow them localize the event. We build on some previous works with centralized way of doing this. We do not foresee any theoretical challenge in doing this. But it remains an interesting topic how to do it in an efficient way.

## 4. Potential Impact

If the project is successful, what difference will it make, both technically and broadly?

## 5. Challenges

What are the challenges and risks?

## 6. Requirements for Success

What skills and resources are necessary to perform the project?

## 7. Metrics of Success

What are metrics by which you would check for success?

## 8. Execution Plan

Describe the key tasks in executing your project, and in case of team project describe how will you partition the tasks.

## 9. Related Work

### 9.a. Papers

List the key papers that you have identified relating to your project idea, and describe how they related to your project. Provide references (with full citation in the References section below).

### 9.b. Datasets

List datasets that you have identified and plan to use. Provide references (with full citation in the References section below).

### 9.c. Software

List softwate that you have identified and plan to use. Provide references (with full citation in the References section below).

## 10. References

List references correspondign to citations in your text above. For papers please include full citation and URL. For datasets and software include name and URL.
