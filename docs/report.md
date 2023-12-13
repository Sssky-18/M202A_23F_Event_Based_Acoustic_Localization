# Table of Contents
* Abstract
* [Introduction](#1-introduction)
* [Related Work](#2-related-work)
* [Technical Approach](#3-technical-approach)
* [Evaluation and Results](#4-evaluation-and-results)
* [Discussion and Conclusions](#5-discussion-and-conclusions)
* [References](#6-references)

# Abstract

Provide a brief overview of the project objhectives, approach, and results.

This project endeavors to transform any given surface into a functional touchscreen through the implementation of synchronized touch sensors. The primary objective involves the establishment of a system wherein three sensors are strategically positioned and synchronized to detect touch events sequentially, correlating with their respective distances from the event location. Utilizing the Time Difference of Arrival (TDOA) method, the project leverages the time variances between sensor pairs to compute the disparities in distances between sensors. This calculation aids in localizing the origin of the touch event by integrating the coordinates of the three sensors and their respective distance variances.

The operational framework encompasses the programming of a server host capable of receiving post requests from the three sensors. Subsequently, the host executes TDOA functions to accurately estimate the location of the event source. This methodology promises to enable any surface to function as a touch-sensitive interface, expanding the possibilities for interactive user experiences across diverse environments.

# 1 Introduction

This section should cover the following items:

* Motivation & Objective: What are you trying to do and why? (plain English without jargon)
* State of the Art & Its Limitations: How is it done today, and what are the limits of current practice?
* Novelty & Rationale: What is new in your approach and why do you think it will be successful?
* Potential Impact: If the project is successful, what difference will it make, both technically and broadly?
* Challenges: What are the challenges and risks?
* Requirements for Success: What skills and resources are necessary to perform the project?
* Metrics of Success: What are metrics by which you would check for success?


## 1.1 Motivation & Objective

A flat surface can be more than just a surface. You can project onto it into a display. In this project we want to look at using everyday devices to turn a surface into touch sensor. More specifically, acoustic devices placed at the vertices pick up a touch event propagated through air and log the time of arrival. Using the time of arrival, we can solve the location of the touch event. This project involves two challenges, one is to solve the location of the touch location given time of arrival, the other is to make devices sync with each other to get accurate time of arrival. 

## 1.2 State of the Art & Its Limitations

For implementing touch interaction on large surface, several ways are avaible and some of them are already commercially viable. One way is to use a camera to track the touch event. However, it requires a camera to be placed above the surface and the camera needs to be calibrated. Also privacy can be harmed. Another way is to use a capacitive touch sensor. This is no different from any small touchscreen. But in the case of large surface, the cost and power consumption of the sensor can be high. Also, the sensor needs to be manufactured to adapt to the size. It is not plug and play. Another way is to use infrared pairs. By looking at which part has been blocked, we can infer the touch event. However, this method requires bulky hardware.

The idea of using acoustic to infer touch event is also popular and leads to some innovative work. In Acustico, the time difference between surface wave and sound wave are used to infer touch location. All equipments are on users's wrist. Since the time differnce is small, high sampling rate is necessary. Also, it provides relative location to hand instead of absolute location on the surface. 

In Toffee, microphoes at different edges of the same device are used and the TDoA is used to inference the relative angle of the touch event. It opens new way of user interaction. However, it does not provide localization on the surface.

In ALTo, the time difference of arrival of the event's acoustic signal at a bunch of microphones are used to infer touch location. The microphones are placed at the vertices of the surface. The microphones are connect to the same device for data collection and processing. We recognize this as very stringent as it does not scale to large surface. Also it prevent the possibility of using a bunch of independent devices to infer touch location. We consider this work as an important preliminary work and we want to extend it to a more practical case.

## 1.3 Novelty & Rationale

We want to use a bunch of independent devices to infer touch location on a large surface. We want to design a protocol between devices to allow them localize the event. We build on some previous works with centralized way of doing this. We do not foresee any theoretical challenge in doing this. But it remains an interesting topic how to do it in an efficient way.

## 1.4 Potential Impact

We will have better understanding in time synchronization (Haochen's part) and localization (Tianyuan's part). Also this may result in a working app on smart phones that can turn any surface into a touch sensor.

## 1.5 Challenges

The audio kit we sourced uses MEMS microphone and small speaker. This may impair the ability to capture event and the ability to propagate information. If this is indeed an issue, we plan to make everything louder.

## 1.6 Requirements for Success

For the hardware development, we need to plan well to allow stable capture of sound to ensure correct timestamping. This requires skills in realtime system developing.

For the software part, we need to design a protocol to allow devices to sync with each other. We also need to implement the algorithm to solve for the location.

## 1.7 Metrics of Success

We aim to get a working demo to present. On the demo we can measure accuracy. If that failed, we fallback to demo timestamping same acoustic event on different devices.

# 2. Related Work

# 3. Technical Approach

# 4. Evaluation and Results

# 5. Discussion and Conclusions

# 6. References
