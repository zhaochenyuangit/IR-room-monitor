'''
in file mode, program exits after reading the last command
in console mode, program exits if reads an empty string (type in enter without any content)
'''
import sys
import serial
import json
import time
import argparse

parser = argparse.ArgumentParser(description=
                                 'interact with ESP32')
parser.add_argument('--port','-p',type=str,default='COM10',
                    help='serial port number')
parser.add_argument('--mode','-m',type=str,choices=['console','file','frame'],
                    help='the path to the entire model file')
parser.add_argument('--filename','-f',type=str,
                    help='event data file')

args = parser.parse_args()

class event_simulator():
    def __init__(self,sysargs):
        self.ser = serial.Serial(sysargs.port,115200)
        if sysargs.mode == 'console':
            self.get_next_cmd = self.get_console_cmd
        elif sysargs.mode == 'file':
            self.filename = sysargs.filename
            f = open(sysargs.filename,'r')
            self.lines = f.readlines()
            f.close()
            self.line_total = len(self.lines)
            self.line_index = 0
            self.get_next_cmd = self.get_file_cmd
        elif sysargs.mode == 'frame':
            self.filename = sysargs.filename
            f = open(sysargs.filename,'r')
            self.lines = f.readlines()
            f.close()
            self.line_total = len(self.lines)
            self.line_index = 0
            self.get_next_cmd = self.get_frame_cmd
            
    def get_console_cmd(self):
        print('please input next command: ',end='')
        cmd = input()
        if not cmd: #enter empty row to exit programm
            print("program terminate")
            exit()
        else:
            cmd = cmd + '\n'
        return cmd
        
    def get_file_cmd(self):
        if self.line_index < self.line_total:
            print("{} frame {}/{}".format(self.filename, self.line_index+1,self.line_total))
            cmd = self.lines[self.line_index]
            self.line_index += 1
        else:
            cmd = self.get_console_cmd()
        return cmd
    
    def get_frame_cmd(self):
        frame = -1
        while not (0<=frame<self.line_total):
            print('choose a frame from 0-{}: '.format(self.line_total-1),end=' ')
            cmd_str = input()
            if cmd_str:
                if cmd_str == "q":
                    print("program terminate")
                    exit()
                frame = int(cmd_str)
                self.line_index = frame
            else:
                print("auto choose next frame {}".format(self.line_index))
                self.line_index += 1
                frame = self.line_index
     
        cmd = self.lines[frame]
        return cmd
            
    def write_cmd(self):           
        cmd = self.get_next_cmd()
        self.ser.write(cmd.encode())
        # wait until command is sent
        while(self.ser.out_waiting):
            pass
            
    def skip_all_buf(self):
        while(self.ser.in_waiting):
            _ = self.ser.readline()
    
    def read_all_buf(self):
        tik = time.time()
        while(self.ser.in_waiting):
            tok = time.time()
            if (tok-tik) > 2:
                break
            info = self.ser.readline()
            if info!=b'':
                print(info.decode('utf-8'))

if __name__ == '__main__':      
    sim = event_simulator(args)
    sim.skip_all_buf()
    while(True):
        sim.write_cmd()
        time.sleep(0.2)
        sim.read_all_buf()