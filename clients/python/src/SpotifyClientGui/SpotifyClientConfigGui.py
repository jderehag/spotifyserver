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
from Tkinter import *
import tkMessageBox
import socket

class SpotifyClientConfigGui(Toplevel):
    def __init__(self, configHandler):
        Toplevel.__init__(self)
        self.title("Config")
        self.protocol("WM_DELETE_WINDOW", self.__del__)
        #self.geometry("300x200")
        self.configHandler = configHandler

        self.labelIpAddress = Label(self, justify=LEFT, text="Server IP:").grid(row=0, column=0, sticky=W)
        self.entryIpAddress = Entry(self)
        self.entryIpAddress.insert(0, self.configHandler.getIpAddr())
        self.entryIpAddress.grid(row=0, column=1, columnspan=2, sticky=W)
        
        self.labelPort = Label(self, justify=LEFT, text="Server Port:").grid(row=1, column=0, sticky=W)
        self.entryPort = Entry(self)
        self.entryPort.insert(0, self.configHandler.getPort())
        self.entryPort.grid(row=1, column=1, columnspan=2, sticky=W)
        
        self.buttonOk = Button(self, text="OK", width=6, command=self.buttonOkClicked).grid(row=2, column=1, sticky=E)
        self.buttonCancel = Button(self, text="Cancel", command=self.buttonCancelClicked).grid(row=2, column=2, sticky=E)
        
        self.bind("<Return>",self.buttonOkClicked)
        self.bind("<Escape>",self.buttonCancelClicked)
        
    def __del__(self):
        self.destroy()
    
    def buttonOkClicked(self, *event):
        
        #Validate ip addr!
        ipaddr = self.entryIpAddress.get()
        try:
            socket.inet_aton(ipaddr)
        except socket.error:
            tkMessageBox.showerror("Invalid IP address", "Invalid ip address, must be in dot notation i.e 127.0.0.1")
            return

        #Validate portnumber!
        try:
            port = int(self.entryPort.get())
            if(port <= 0 or port >= 65535):
                tkMessageBox.showerror("Invalid port", "Port must be a number between 1 and 65535")
                return
        except ValueError:
            tkMessageBox.showerror("Invalid port", "Port %s is not an integer" % self.entryPort.get())
            return
        
        self.configHandler.setIpAddr(ipaddr)
        self.configHandler.setPort(port)
        try:
            self.configHandler.writeToFile()
        except:
            tkMessageBox.showerror("File error", "Could not save settings to file!")
            return
        
        self.configHandler.configUpdateDoneInd()
        self.destroy()
        return
    
    def buttonCancelClicked(self, *event):
        self.destroy()