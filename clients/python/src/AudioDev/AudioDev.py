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

try:
    import pyaudio
except:
    pass

from threading import Thread, Event

import wave
import Queue

MIC_FORMAT = pyaudio.paInt16 
MIC_CHANNELS = 1
MIC_RATE = 44100  
MIC_INPUT_BLOCK_TIME = 0.05
MIC_INPUT_FRAMES_PER_BLOCK = int(MIC_RATE*MIC_INPUT_BLOCK_TIME)


class AudioDev(Thread):
    def __init__(self, useMic = False, audioCallback=None):
        Thread.__init__(self)
        self._pa = pyaudio.PyAudio()
        self._useMic = useMic
        
        self._output_stream = None
        self._channels = 0
        self._rate = 0
        self._format = 0
        self._output_fifo = Queue.Queue(0)
        
        if useMic:
            self._cancellationPending = Event()
            self._audioCallback = audioCallback
        
            self._stream = self.open_mic_stream()
            self.daemon = True
            self.start()

    def __del__(self):
        if self._output_stream != None:
            self._output_stream.close()
        self._pa.terminate()
        
    def stop(self):
        if self._useMic:
            self.stream.close()

    def open_mic_stream(self):
        assert self._useMic == True
        
        device_index = self.find_input_device()
        stream = self._pa.open(format = MIC_FORMAT,
                              channels = MIC_CHANNELS,
                              rate = MIC_RATE,
                              input = True,
                              input_device_index = device_index,
                              frames_per_buffer = MIC_INPUT_FRAMES_PER_BLOCK)

        return stream
    
    
    def open_output_stream(self, channels, rate, format_ = pyaudio.paInt32):
        # self.close_output_stream()
        # open stream based on the wave object which has been input.
        # format = paFloat32, paInt32, paInt24, paInt16, paInt8, paUInt8, paCustomFormat
        self._output_stream = self._pa.open(format = format_,
                               channels = channels,
                               rate = rate,
                               output = True, 
                               stream_callback=self._get_next_data_for_outputstream)
        
        self._channels = channels
        self._rate = rate
        self._format = format
        
    
    def close_output_stream(self):
        if self._output_stream != None:
            self._output_stream.stop_stream()

    def write_to_output_stream(self, channels, rate, nsamples, data):
        if channels != self._channels or rate != self._rate:
            self.open_output_stream(channels, rate)
        
        sampleSize = 2
        totalSampleSize = sampleSize * channels
        
        for n in range(0, nsamples):
            sample = str(data[n * totalSampleSize : ((n+1) * totalSampleSize)])
            self._output_fifo.put(sample)
        
        
        if not self._output_stream.is_active():
            if self._output_fifo.qsize() > 100000:
                self._output_stream.start_stream()

        
                
    def find_input_device(self):
        assert self._useMic == True
        
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
                self._audioCallback(self._stream.read(MIC_INPUT_FRAMES_PER_BLOCK))
            except IOError, e:
                print( "(%d) Error recording: %s"%(self.errorcount,e) )
        
    def _get_next_data_for_outputstream(self, data, requested_samples, time_info, status):        
        samples = []
        for n in range(requested_samples):
            try:
                samples.append(self._output_fifo.get(block=True, timeout=1))
                self._output_fifo.task_done()
            except Queue.Empty:
                print "JESPER timedout waiting for more audio data!"
                return (None, pyaudio.paComplete)
        print "JESPER, returning", len(samples), "samples, requested=", requested_samples, "qsize=", self._output_fifo.qsize()
        return (str(samples), pyaudio.paContinue)
        
        

                        