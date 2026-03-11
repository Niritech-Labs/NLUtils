# Copyright (C) 2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
from .Logger import NLLogger,ConColors
from .Parser import NLParser
from .BaseParserRealizations import TargetsParser
from .BlocksUtils import Blocks
from pathlib import Path
import shutil, subprocess
class NLInstaller:
    def __init__(self,pathToTargets: str,root:str | None = None):
        self.path = str(Path(pathToTargets).expanduser())
        self.Logger = NLLogger(True,'Installer')
        self.targets = []

        if root:
            self.root = root
        else:
            self.root = ''

        self.Parser = NLParser(True)
        
        self.Parser.SetParserRealization('targets',TargetsParser)
        self.Data:Blocks = self.Parser.OpenFile(self.path,'targets').Read()



    def RunTarget(self,name:str):
        blocks = self.Data.FindBlock(name)
        if blocks:
            blockName = blocks[0].name
            targets = blocks[0].GetAllParams()
            self.Logger.Info(f'Running Target {blockName}',ConColors.B,True)
            
            for target in targets:
                # command handler
                if target[0] == 'command':
                    if target[1].count('-o-') == 1:
                        command, priority = target[1].split('-o-')
                        self.RunCommand(command,priority)
                    else:
                        self.Logger.Warning(f'A broken line was found in the target: {blockName}')
                # print handler
                elif target[0] == 'print':
                    self.Logger.Info(target[1],ConColors.V,True)
                # file handler
                elif target[0] == 'target':
                    if target[1].count('-o-') == 3:
                        destination,source,opType,access = target[1].split('-o-')
                        dst = Path(self.root + str(Path(destination).expanduser()))
                        src = Path(source)
                        mode = int(access,8)

                        if src.is_dir():
                            if opType == 'install':
                                shutil.copytree(str(source),str(dst))
                                dst.chmod(mode)
                            elif opType == 'remove':
                                if dst.exists():
                                    shutil.rmtree(str(dst))
                                else:
                                    self.Logger.Warning(f'Dir not found: {str(dst)}')
                            
                        elif src.is_file():
                            if opType == 'install':
                                shutil.copy2(str(source),str(dst))
                                dst.chmod(mode)
                            elif opType == 'remove':
                                if dst.exists():
                                    dst.unlink()
                                else:
                                    self.Logger.Warning(f'File not found: {str(dst)}')  
                    else:
                        self.Logger.Warning(f'A broken line was found in the target: {blockName}')
                # link handler
                elif target[0] == 'links':
                    if target[1].count('-o-') == 3:
                        destination,linkDestination,opType,linkType = target[1].split('-o-')
                        dst = Path(self.root + str(Path(destination).expanduser()))
                        ldst = Path(self.root + str(Path(linkDestination).expanduser()))
                        if opType == 'install':
                            if linkType == 'hardlink':
                                ldst.hardlink_to(dst)
                            elif linkType == 'symlink':
                                ldst.symlink_to(dst)
                        elif opType == 'remove':
                            if ldst.exists():
                                ldst.unlink()
                            else:
                                self.Logger.Warning(f'File not found: {str(dst)}')  
                    else:
                        self.Logger.Warning(f'A broken line was found in the target: {blockName}')
                #dir handler
                elif target[0] == 'directory':
                    if target[1].count('-o-') == 2:
                        destination,opType,access = target[1].split('-o-')
                        dst = Path(self.root + str(Path(destination).expanduser()))
                        mode = int(access,8)
                            
                        if opType == 'install':
                            dst.mkdir()
                            dst.chmod(mode)
                        elif opType == 'remove':
                            if dst.exists():
                                shutil.rmtree(str(dst))
                            else:
                                self.Logger.Warning(f'Dir not found: {str(dst)}')
                    else:
                        self.Logger.Warning(f'A broken line was found in the target: {blockName}')


    def RunCommand(self,command:str,type:str):
        try:
            self.Logger.Info(f"Running command {command}",ConColors.G,True)
            cmd = command.split(' ')
            result = subprocess.run(cmd,check=True)
        except subprocess.CalledProcessError as E:
            if type == 'critical':
                self.Logger.Error(str(E),True)
            if type == 'error':
                self.Logger.Error(str(E),False)
            if type == 'warning':
                self.Logger.Warning(str(E))
        except Exception as E:
            self.Logger.Error(str(E),True)



    



