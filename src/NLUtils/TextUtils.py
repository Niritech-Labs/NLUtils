# Copyright (C) 2024-2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
from .Logger import NLLogger
import re

class GigaRE:
    def __init__(self,production):
        self.Logger = NLLogger(production,"GigaRE")

# -o-

    def filtering(self,blacklist,sentence):
        for el in blacklist:
            if el in sentence:
                return True 
        return False

    def FindAndWrap(self,text:str,re_pattern:str,handleFunction,wrapSkeleton:str,blacklist:list[str] = []) -> list:
        self.Logger.Info(f'start finding pattern:{re_pattern}',NLLogger.ConColors.G,False)
        pattern = re.compile(re_pattern,re.DOTALL)
        patternResults = [m.group() for m in pattern.finditer(text)]
        result = []
        self.Logger.Info(f'finded:{len(patternResults)}',NLLogger.ConColors.G,False)
        for patternResult in patternResults:
            self.Logger.Info(f'processing:{patternResult}',NLLogger.ConColors.B,False)
            if self.filtering(blacklist,patternResult):
                self.Logger.Info('Filtered',NLLogger.ConColors.G,False)
                continue

            postprocessingResult = handleFunction(patternResult)
            if not postprocessingResult[0]:
                self.Logger.Info('Not valid',NLLogger.ConColors.G,False)
                continue
            
            wrap = wrapSkeleton
            for i in range(len(postprocessingResult)-1):
                prt1,prt2 = wrap.split(f'-o-key{i+1}-o-')
                wrap = prt1 + postprocessingResult[i+1] + prt2
            result.append(wrap)
        return result
                






