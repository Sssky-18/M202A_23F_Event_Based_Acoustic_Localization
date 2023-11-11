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

We will have better understanding in time synchronization (Haochen's part) and localization (Tianyuan's part). Also this may result in a working app on smart phones that can turn any surface into a touch sensor.

## 5. Challenges

The audio kit we sourced uses MEMS microphone and small speaker. This may impair the ability to capture event and the ability to propagate information. If this is indeed an issue, we plan to make everything louder.

## 6. Requirements for Success

For the hardware development, we need to plan well to allow stable capture of sound to ensure correct timestamping. This requires skills in realtime system developing.

For the software part, we need to design a protocol to allow devices to sync with each other. We also need to implement the algorithm to solve for the location.

## 7. Metrics of Success

We aim to get a working demo to present. On the demo we can measure accuracy. If that failed, we fallback to demo timestamping same acoustic event on different devices.

## 8. Execution Plan

Haochen Zhao will be focusing on the hardware and protocol part. Tianyuan Nan will be focusing on the algorithm and github part. We will work together on the demo.

Key tasks:
* Haochen Zhao: audio capture routine
* Haochen Zhao: audio playback routine (for sync using sound)
* Haochen Zhao: acoutic event timestamping
* Haochen Zhao: protocol design
* Tianyuan Nan: algorithm design
* Tianyuan Nan: github repo
* Tianyuan Nan: host software for data processing

## 9. Related Work

### 9.a. Papers

[Acustico: Surface Tap Detection and Localization using Wrist-based Acoustic TDOA Sensing](https://dl.acm.org/doi/10.1145/3379337.3415901) In Acustico, the time difference between surface wave and sound wave are used to infer touch location. All equipments are on users's wrist. Since the time differnce is small, high sampling rate is necessary. Also, it provides relative location to hand instead of absolute location on the surface. 

[ALTo: Ad Hoc High-Accuracy Touch Interaction Using Acoustic Localization](https://arxiv.org/abs/2108.06837) In Toffee, microphoes at different edges of the same device are used and the TDoA is used to inference the relative angle of the touch event. It opens new way of user interaction. However, it does not provide localization on the surface.

[Toffee: enabling ad hoc, around-device interaction with acoustic time-of-arrival correlation](https://dl.acm.org/doi/10.1145/2628363.2628383) In ALTo, the time difference of arrival of the event's acoustic signal at a bunch of microphones are used to infer touch location. The microphones are placed at the vertices of the surface. The microphones are connect to the same device for data collection and processing. We recognize this as very stringent as it does not scale to large surface. Also it prevent the possibility of using a bunch of independent devices to infer touch location. We consider this work as an important preliminary work and we want to extend it to a more practical case.

### 9.b. Datasets

Not applicable yet.

### 9.c. Software

Not applicable yet.

## 10. References

* Jun Gong, Aakar Gupta, and Hrvoje Benko. 2020. Acustico: Surface Tap Detection and Localization using Wrist-based Acoustic TDOA Sensing. In Proceedings of the 33rd Annual ACM Symposium on User Interface Software and Technology (UIST '20). Association for Computing Machinery, New York, NY, USA, 406–419. (https://doi.org/10.1145/3379337.3415901)
* Seshan, Arvind. "ALTo: Ad Hoc High-Accuracy Touch Interaction Using Acoustic Localization." arXiv preprint arXiv:2108.06837 (2021). (https://arxiv.org/abs/2108.06837)
* Robert Xiao, Greg Lew, James Marsanico, Divya Hariharan, Scott Hudson, and Chris Harrison. 2014. Toffee: enabling ad hoc, around-device interaction with acoustic time-of-arrival correlation. In Proceedings of the 16th international conference on Human-computer interaction with mobile devices &amp; services (MobileHCI '14). Association for Computing Machinery, New York, NY, USA, 67–76. (https://doi.org/10.1145/2628363.2628383)


List references correspondign to citations in your text above. For papers please include full citation and URL. For datasets and software include name and URL.
