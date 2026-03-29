# Copyright (C) 2024-2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

class ConColors: 
    R = "\033[91m"
    G = "\033[92m"
    Y = "\033[93m"
    B = "\033[94m"
    V = "\033[95m"
    S = "\033[0m"


class NLLogger:
    class ConColors: 
        R = "\033[91m"
        G = "\033[92m"
        Y = "\033[93m"
        B = "\033[94m"
        V = "\033[95m"
        S = "\033[0m"
    def __init__(self, production: bool, ComponentName: str = '',logList:list | None = None):
        self.production = production
        self.toLogList = False

        if not logList is None:
            self.logList = logList
            self.toLogList = True
        self.name = " " + ComponentName 

    def Warning(self,warn: str):
        if self.toLogList:
            self.logList.append(f"{ConColors.Y} Warning{self.name}: {warn}{ConColors.S}")
        print(f"{ConColors.Y} Warning{self.name}: {warn}{ConColors.S}")


    def Error(self,err:str,critical:bool):
        if critical:
            if self.toLogList:
                self.logList.append(f"{ConColors.R} Critical Error{self.name}: {err}{ConColors.S}")
            print(f"{ConColors.R} Critical Error{self.name}: {err}{ConColors.S}")
            exit(1)
        else:
            if self.toLogList:
                self.logList.append(f"{ConColors.R} Error{self.name}: {err}{ConColors.S}")
            print(f"{ConColors.R} Error{self.name}: {err}{ConColors.S}")
    
    def Info(self,inf:str,color: ConColors | object, productionLatency: bool):
        if self.production:
            if productionLatency: 
                if self.toLogList:
                    self.logList.append(f"{color} Info{self.name}: {inf}{ConColors.S}")
                print(f"{color} Info{self.name}: {inf}{ConColors.S}")
        else:
            if self.toLogList:
                self.logList.append(f"{color} Info{self.name}: {inf}{ConColors.S}")
            print(f"{color} Info{self.name}: {inf}{ConColors.S}")