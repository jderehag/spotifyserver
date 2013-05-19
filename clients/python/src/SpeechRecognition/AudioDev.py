'''
Copyright (c) 2012, Jesper Derehag
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
  * Neither the name of the <organization> nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL JESPER DEREHAG BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''


import pyaudio

from threading import Thread, Event

FORMAT = pyaudio.paInt16 
CHANNELS = 1
RATE = 44100  
INPUT_BLOCK_TIME = 0.05
INPUT_FRAMES_PER_BLOCK = int(RATE*INPUT_BLOCK_TIME)


class AudioDev(Thread):
    def __init__(self, audioCallback):
        Thread.__init__(self)
        self._pa = pyaudio.PyAudio()
        self._stream = self.open_mic_stream()
        self._cancellationPending = Event()
        self._audioCallback = audioCallback
        self.daemon = True
        self.start()
        
    def stop(self):
        self.stream.close()

    def open_mic_stream( self ):
        device_index = self.find_input_device()
        stream = self._pa.open(format = FORMAT,
                              channels = CHANNELS,
                              rate = RATE,
                              input = True,
                              input_device_index = device_index,
                              frames_per_buffer = INPUT_FRAMES_PER_BLOCK)

        return stream
    
    def find_input_device(self):
        device_index = None            
        for i in range( self._pa.get_device_count() ):     
            devinfo = self._pa.get_device_info_by_index(i)   
            print( "Device %d: %s"%(i,devinfo["name"]) )

            for keyword in ["mic","input"]:
                if keyword in devinfo["name"].lower():
                    print( "Found an input: device %d - %s"%(i,devinfo["name"]) )
                    device_index = i
                    return device_index

        if device_index == None:
            print( "No preferred input found; using default input device." )

        return device_index
    
    def run(self):
        while (self._cancellationPending.is_set() == False):
            try:
                self._audioCallback(self._stream.read(INPUT_FRAMES_PER_BLOCK))
            except IOError, e:
                print( "(%d) Error recording: %s"%(self.errorcount,e) )
            

                        