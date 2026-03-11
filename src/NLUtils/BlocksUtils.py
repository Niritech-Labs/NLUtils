# Copyright (C) 2024-2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
import re

class Block:
    def __init__(self,name:str):
        self.name = name
        self.blocks:list[Block] = []
        self.params:list[list] = []
        self.level = 1

    def FindParamRecursive(self,name:str) -> None | list:
        """Recursive find"""
        params = []
        for param in self.params:
            if param[0] == name:
                params.append(param)

        for block in self.blocks:
            rparams = block.FindParamRecursive(name)
            if rparams:
                params += rparams

        if params:
            return params
        else:
            return None
        
    def FindParam(self,name:str) -> None | list:
        """Recursive find"""
        params = []
        for param in self.params:
            if param[0] == name:
                params.append(param)

        if params:
            return params
        else:
            return None
            
    def FindBlockRecursive(self,name:str) -> None | list:
        """Recursive find"""
        blocks = []
        for block in self.blocks:
            rblocks = block.FindBlockRecursive(name)
            if rblocks:
                blocks += rblocks
            if block.name == name:
                blocks.append(block)
        
        if blocks:
            return blocks
        else:
            return None
        
    def DeleteMarkedObjects(self):
        for param in list(self.params):
            if param[0] == None:
                self.params.remove(param)
        for block in list(self.blocks):
            block.DeleteMarkedObjects()
            if block.name == None:
                self.blocks.remove(block)
        


        
    def FindBlock(self,name:str) -> None | list:
        blocks = []
        for block in self.blocks:
            if block.name == name:
                blocks.append(block)
        
        if blocks:
            return blocks
        else:
            return None
        
    def AddBlock(self,block):
        block.level += self.level
        self.blocks.append(block)

    def AddParam(self,key,value):
        self.params.append([key,value])


    def DeleteAllBlocks(self):
        self.blocks = []

    def GetAllBlocks(self) -> list:
        return self.blocks

    def GetAllParams(self) -> list:
        return self.params
    
    def SetBlocks(self,blocks:list['Block']):
        self.blocks = blocks

    def SetParams(self,params:list[list[object]]):
        self.params = params

    def DeleteAllParams(self):
        self.params = []

    def resetLevel(self):
        self.level = 1
        for block in self.blocks:
            block.resetLevel()

    def updateLevel(self):
        for block in self.blocks:
            block.level += self.level
            block.updateLevel()

    def GetName(self) -> str:
        return self.name

            
    def ToStr(self)-> str:
        blockTabs = '' if self.level == 1 else '        '*(self.level-1)
        
        blocks = f"{blockTabs}name: {self.name}\n"

        if not self.params == []:
            blocks += f'{blockTabs}    params:\n'
            for param in self.params:
                blocks += f'{blockTabs}        {param[0]} = {param[1]}\n'
            blocks += '\n'

        if not self.blocks == []:
            blocks += f'{blockTabs}    blocks:\n'
            for block in self.blocks:
                blocks += block.ToStr()
            blocks += '\n'

        return blocks






class Blocks(Block):
    def __init__(self,name:str):
        super().__init__(name)
   
    @staticmethod
    def FromStr(data: str) -> 'Blocks':
        lines = data.strip().split('\n')
    
        while lines and not lines[0].strip():
            lines.pop(0)
        while lines and not lines[-1].strip():
            lines.pop()
    
        if not lines or not lines[0].strip().startswith('name:'):
            raise ValueError('*.blocks string invalid')
    
        
        mainBlockName = lines[0].strip().split(':', 1)[1].strip()
        root = Blocks(mainBlockName)
    
        stack = [(0, root)]
    
        sections = [None] 
    
        namePattern = re.compile(r'^\s*name:\s*(.+)$')
        paramsEntryPattern = re.compile(r'^\s*params:\s*$')
        blocksEntryPattern = re.compile(r'^\s*blocks:\s*$')
        paramPattern = re.compile(r'^\s*(\S+)\s*=\s*(.+)$')
    
        for _, line in enumerate(lines[1:], start=2):
            indent = len(line) - len(line.lstrip(' '))
            content = line.strip()
            if not content:
                continue
            
            while stack and stack[-1][0] >= indent:
                stack.pop()
                sections.pop()
        

            nameMatch = namePattern.match(line)
            if nameMatch:
                blockName = nameMatch.group(1).strip()
                newBlock = Block(blockName)
            
                if stack:
                    parent = stack[-1][1]
                    parent.AddBlock(newBlock)
                else:
                    root.AddBlock(newBlock)
            
                stack.append((indent, newBlock))
                sections.append(None)
                continue
        
            if paramsEntryPattern.match(line):
                if stack:
                    sections[-1] = 'params'
                continue
        
            if blocksEntryPattern.match(line):
                if stack:
                    sections[-1] = 'blocks'
                continue
        

            if stack and sections[-1] == 'params':
                paramMatch = paramPattern.match(line)
                if paramMatch:
                    key = paramMatch.group(1).strip()
                    value = paramMatch.group(2).strip()
                
                    current_block = stack[-1][1]
                    current_block.AddParam([key, value])
                    continue
        
            paramMatch = paramPattern.match(line)
            if paramMatch and indent == 0 and len(stack) == 1:
                key = paramMatch.group(1).strip()
                value = paramMatch.group(2).strip()
              
                root.AddParam([key, value])
                continue
    
        return root

    def resetLevel(self):
        self.level = 1
        for block in self.blocks:
            block.resetLevel()

    def UpdateLevel(self):
        self.resetLevel()
        for block in self.blocks:
            block.level += self.level
            block.updateLevel()



            
    def ToStr(self) -> str:
        blockTabs = '' if self.level == 1 else '        '*(self.level-1)
        
        blocks = f"{blockTabs}name: {self.name}\n"

        if not self.params == []:
            blocks += f'{blockTabs}    params:\n'
            for param in self.params:
                blocks += f'{blockTabs}        {param[0]} = {param[1]}\n'
            blocks += '\n'

        if not self.blocks == []:
            blocks += f'{blockTabs}    blocks:\n'
            for block in self.blocks:
                blocks += block.ToStr()
            blocks += '\n'

        return blocks

    def AddNewRootBlock(self,name):
        block = Block(self.name)
        block.params = self.params
        block.blocks = self.blocks
        self.params = []
        self.blocks = []
        self.name = name
        self.AddBlock(block)
        self.UpdateLevel()




    

