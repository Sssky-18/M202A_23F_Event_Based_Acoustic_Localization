# Abstract

This project endeavors to transform any given surface into a functional touchscreen through the implementation of synchronized touch sensors. The primary objective involves the establishment of a system wherein three sensors are strategically positioned and synchronized to detect touch events sequentially, correlating with their respective distances from the event location. Utilizing the Time Difference of Arrival (TDOA) method, the project leverages the time variances between sensor pairs to compute the disparities in distances between sensors. This calculation aids in localizing the origin of the touch event by integrating the coordinates of the three sensors and their respective distance variances. The operational framework encompasses the programming of a server host capable of receiving post requests from the three sensors. Subsequently, the host computer executes TDOA functions to accurately estimate the location of the event source. This methodology promises to enable any surface to function as a touch-sensitive interface, expanding the possibilities for interactive user experiences across diverse environments.

After conducting the experiment, the firmware effectively transmits the necessary data to the server computer via a Wi-Fi network. This transmitted data is then utilized in calculating the time differences required for the Time Difference of Arrival (TDOA) functions. Consequently, these functions successfully incorporate this data as one of their inputs, culminating in the provision of a reasonable and accurate estimated location for the event source. The estimated location exhibits a standard deviation of up to 0.04 units from the actual location in both axes. This marginal deviation signifies the efficiency of the entire process.
# Team

* Name of team member \#1 **Haochen Zhao**
* Name of team member \#2 **Tianyuan Nan**


# Required Submissions

* [Proposal](proposal.md)
* [Midterm Checkpoint Presentation Slides](media/Mid_Slides.pdf)
* [Final Presentation Slides](media/Final_Slide_Event_Based_Acoustic_Location.pdf)
* [Final Report](report.md)
