# Copyright (C) 2024-2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
from .Logger import NLLogger,ConColors
from pathlib import Path
from .BlocksUtils import Blocks

class ParserRealizationFabric:
    def __init__(self,production:bool,path:str,name:str):
        self.LOG = NLLogger(production,name)
        self.path = Path(path).expanduser().resolve()

    def open(self):
        try:
            with open(self.path, 'r', encoding='utf-8') as file:
                return file.read()
        except:
            self.LOG.Error(f"Can't load this file:{self.path}",False)
            return None
        
    def save(self,data:str):
        try:
            with open(self.path, 'w', encoding='utf-8') as file:
                file.write(data)

        except Exception as e:
            self.LOG.Error(str(e)+f", Can't save this file:{str(self.path)}",False)

class NLParserObject:
    def __init__(self,realization,filepath):
        self.Realization = realization
        self.filepath = filepath

    def Read(self) -> Blocks | None:
        return self.Realization.Decode()
    def Write(self,data:Blocks):
        self.Realization.Encode(data)

class NLParser:
    def __init__(self,production:bool):
        self.production = production
        self.LOG = NLLogger(production,'NLParser')
        self.LOG.Info('Started',ConColors.G,False)
        self.realization = {}

    def SetParserRealization(self,name:str,realization:type[ParserRealizationFabric]):
        self.checkRealization(realization,name)

    def checkRealization(self,realization,name):
        if realization == ParserRealizationFabric:
            self.LOG.Error('Please use real realization, not basic class',True)
        if not (hasattr(realization,'Encode') and callable(getattr(realization,'Encode')) and hasattr(realization,'Decode') and callable(getattr(realization,'Decode'))):
            self.LOG.Error('Please use valid realization',True)
        else:
            self.realization[name] = realization
    
    def OpenFile(self,path:str | Path,name:str) -> NLParserObject | None:
        if name in self.realization:
            Realization = self.realization[name](self.production,str(path))
            return NLParserObject(Realization,path)
        else:
            return None
    


