# Copyright (C) 2024-2026 Niritech Labs
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
from .Logger import NLLogger,ConColors
from .JSONUtils import ConfigManager
import os

class NLTranslator:
    def __init__(self,production:bool,language:str,WRITEMODE = False):
        self.writemode = WRITEMODE

        self.rootpath = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

        self.LOG = NLLogger(production,'NLTranslator')
        self.CM = ConfigManager(self.rootpath+'/Settings.confJs',production)
        if language == 'Config':
            self.language = self.CM.LoadConfig()['language']
        else:
            self.language = language

        self.LOG.Info('Started',ConColors.G,False)

        self.Translation = {}
        self.loadTranslation()

    def loadTranslation(self):
        self.Translation = self.CM.OpenRestricted(self.rootpath+'/Translations/'+self.language+'.ntrl')

    def Translate(self,entry:str):
        try:
            return self.Translation[entry]
        except Exception as e:
            if not self.writemode:
                self.LOG.Error(str(e)+f", Can't load translation for this entry: {entry}",False)
                return entry
            else:
                self.Translation[entry] = 'writen'
                self.LOG.Info(f'Entry {entry} writen successfuly',ConColors.B,False)
                return 'writen'