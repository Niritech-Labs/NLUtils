# Copyright (C) 2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
from .Logger import NLLogger,ConColors
from pathlib import Path
import shutil
class NLInstaller:
    def __init__(self,pathToTargets: str):
        self.path = str(Path(pathToTargets).expanduser())
        self.Logger = NLLogger(True,'Installer')
        self.targets = []

        with open(self.path,'r') as f:
            lines = f.readlines()
            for line in lines:
                if not line.startswith('#'):
                    if line.startswith('%'):
                        self.targets.append(line.replace('%','',1))
                    elif line.startswith('/') or line.startswith('~'):
                        if line.count(',') == 2:
                            self.targets.append(line.split(','))
                        else:
                            self.Logger.Warning(f'A broken line was found in the file: {lines.index(line)+1}')
    
    def Install(self):
        try:
            self.Logger.name = ' Installing'
            for target in self.targets:
                if isinstance(target,str):
                    self.Logger.Info(target,ConColors.B,True)

                elif isinstance(target,list):
                    source = Path(target[1])
                    destin = Path(target[0]).expanduser().joinpath(source.name)
                    mode = int(target[2],8)

                    self.Logger.Info(f'{str(source)} -> {target[0]}. Prems: {target[2]}',ConColors.G,True)

                    if source.is_dir():
                        shutil.copytree(str(source),str(destin))
                        destin.chmod(mode)
                    elif source.is_file():
                        shutil.copy2(str(source),str(destin))
                        destin.chmod(mode)

                    
        except Exception as E:
            self.Logger.name = ' Installer'
            self.Logger.Error(str(E),True)


    def Uninstall(self):
        try:
            self.Logger.name = ' Uninstalling'
            for target in self.targets:
                if isinstance(target,str):
                    self.Logger.Info(target,ConColors.B,True)

                elif isinstance(target,list):
                    source = Path(target[1])
                    destin = Path(target[0]).expanduser().joinpath(source.name)
                    
                    self.Logger.Info(f'{str(destin)}',ConColors.G,True)

                    if source.is_dir():
                        shutil.rmtree(str(destin))
                    elif source.is_file():
                        destin.unlink()

                    
        except Exception as E:
            self.Logger.name = ' Installer'
            self.Logger.Error(str(E),True)


inst = NLInstaller('install.targets')
inst.Uninstall()
            
    



