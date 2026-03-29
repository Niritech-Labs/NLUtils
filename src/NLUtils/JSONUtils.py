# Copyright (C) 2024-2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
import json
from pathlib import Path
from NLUtils.Logger import NLLogger, ConColors
class ConfigManager():
    def __init__(self,path,production:bool):
        self.LOG = NLLogger(production,"ConfigManager")
        self.configPath = Path(path).expanduser().resolve()

    def LoadConfig(self) -> dict: 
        try:
            with open(self.configPath, 'r', encoding='utf-8') as file:
                return json.load(file) 
        except:
            self.LOG.Info("Can't load saved config,creatig new",ConColors.S,False)
            defconf = {}
            self.SaveConfig(defconf)
            return defconf
    
# -o-
    def OpenRestricted(self,path):
        path = Path(path).expanduser().resolve()
        try:
            with open(path, 'r', encoding='utf-8') as file:
                return json.load(file) 
        except:
            self.LOG.Error(f"Can't load this config:{path}",False)
            return None
        
    def SaveRestricted(self,path:str,dataToSave:dict):
        try:
            resPath = Path(path).expanduser().resolve()
            if not resPath.exists(): resPath.parent.mkdir(parents=True,exist_ok=True)
            with open(resPath, 'w', encoding='utf-8') as file:
                json.dump(dataToSave, file, ensure_ascii=False, indent=4)

            self.LOG.Info(f'Saved restricted config {str(resPath)}',ConColors.V,False)
        except Exception as e:
            self.LOG.Error(str(e)+f", Can't save this config:{str(resPath)}",False)

    def SaveConfig(self,dataToSave):
        try:
            if not self.configPath.exists(): self.configPath.parent.mkdir(parents=True,exist_ok=True)
            with open(self.configPath, 'w', encoding='utf-8') as file:
                json.dump(dataToSave, file, ensure_ascii=False, indent=4)
            self.LOG.Info(f'Saved main config {str(self.configPath)}',ConColors.V,False)
        except Exception as e:
            self.LOG.Error(str(e)+f", Can't save this config:{str(self.configPath)}",False)
