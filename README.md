This is a collection of DSP Plugins I made. They were mostly made in connection with other projects and helped me gain a better understanding of DSP coding.

There might be better ways of achieving the desired result and I'm open to inputs or other feedback.

<H2>Building</H2>
To build you have to get the Fmod source code, namely the inc folder and place it into the root of a clone of this repository.
With the cmd of your choice go to the repository and then:
```
cmake -S <desiredplugin> -B Builds
cd Builds
make
```
