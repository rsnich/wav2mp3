wav2mp3 convertor WAV files to MP3
----------------------------------

The application accepts a path to a directory with WAV files."

    wav2mp3 <directory with WAV files>"
  
All found WAV files will be converted to MP3 files and 
saved with the same name and in the same directory.


====================================================================================
Build for Linux
====================================================================================

1. Create a directory in the root directory with sources.
    mkdir build

2. Go to the directory.
    cd ./build

3. Use cmake command to generate make files.
    cmake ..                      - Host architecture will be used by default.
    cmake .. -DARCH="x86"         - If it is is necessary to build a binary for x86 architecture.
    cmake .. -DARCH="x86_64"      - If it is is necessary to build a binary for x86_64 architecture.

After successful building wav2mp3 binary is located in "build" directory.

====================================================================================
Build for Windows
====================================================================================

In order to build the application for Windows it is neccessary to VS 2019.

1. Open following solution file in VS 2019 "./vs/wav2mp3.sln".

2. Select necessary architecture and configuration and build the binary.

After successful building wav2mp3 binary is located in "build" directory.
