Feel free to share your infrared transmitter configuration.
This allows to save time to the future user of this git.
And also, to allow those who are not familiar with this kind of manipulation to easily activate their emitter.

If you want to contribute to this project, here is what to do: 

* Case 1 : you have used a configuration present in `config.txt` on a computer not yet referenced  
  * You can add the reference of your computer in the line corresponding to the configuration. 

* Case 2 : you have found a configuration yourself by following the steps described because no configuration of `config.txt` works for you.
  * You can add at the end of the `config.txt` file, the parameters that are different from those already present in the file.
    And indicate above the reference of your computer
  * You can add in the `auto/config.yaml` file each parameter of the script 
    (except the camera path, as I always saw /dev/video2, but I can make it modifiable if needed).
    Use the json structure of the other configurations already present in the file. 
    This file is used for automatic setup. 
    
Fork the git and send me a pull request. Or if you prefer, because it is easier, open an issue. 

By the way, my English is not the best, if some sentences in the readme disturb you, don't hesitate to correct them! 

Thank you for your help ! 
