# A simple utils for NL ecosystem

# RU

## NLUtils.Parser

### `ParserRealizationFabric(production, path, name)`

Абстрактный базовый класс для всех реализаций парсеров.

* **open()**: Внутренний метод для безопасного чтения сырых данных из файла.
* **save()**: Внутренний метод для безопасной записи сырых данных в файл.
* **Decode()**: Абстрактный метод, который должен быть переопределен в наследниках для превращения текста в `Blocks`.
* **Encode()**: Абстрактный метод, который должен быть переопределен в наследниках для превращения `Blocks` в сырой текст.

### `NLParser(production)`

Центральный менеджер парсинга. Он хранит в себе словарь доступных «реализаций» и связывает их с типами файлов.

#### Методы

* **SetParserRealization(name, realizationClass)**: Регистрация нового типа парсера. Позволяет расширять возможности системы без правки ядра.
* **OpenFile(path, type)**: Подготавливает файл к чтению. Находит нужную реализацию по имени `type`, создает её экземпляр и возвращает его.

#### Пример использования в коде

```python
from NLUtils.Parser import NLParser
from NLUtils.BaseParserRealizations import TargetsParser, BlocksParser

# Инициализация менеджера
Parser = NLParser(production=True)

# Регистрация типов парсеров
Parser.SetParserRealization('targets', TargetsParser)
Parser.SetParserRealization('blocks', BlocksParser)

# Чтение файла (интерфейс всегда .Read())
# Возвращает объект типа Blocks
data = Parser.OpenFile('install.targets', 'targets').Read()

# Теперь с данными можно работать через стандартный API Blocks
print(data.GetName())

```


## NLUtils.BaseParserRealizations
### `TargetsParser(production, path)`
Реализация парсера для линейных файлов `.targets`. Использует префиксы.
### `BlocksParser:(production, path)`
Реализация парсера для иерархичных файлов `.blocks`. Мультиформат
### `HyprlangParser:(pathToTargets,root)`
Реализация парсера для файлов Hyprland


## NLUtils.Installer
### `NLInstaller(production, path)`
* **RunTarget(name)**: Исполняет все инструкции внутри указанного блока `%name`.
* **RunCommand(command, type)**: Выполняет системный вызов. Типы: `critical` (стоп), `error`, `warning`.


#### Примеры синтаксиса `.targets`

| Маркер | Имя | Формат значения (`-o-` разделитель) | 
| --- | --- | --- | 
| `%` | **Название блока** | `%Название` |
| `$` | **Запускает команду** | `команда -o- как реагировать на ошибку(critical,error,warning)` |
| `TRG:` | **Создаёт/удаляет файл** | `куда -o- откуда -o- установка/удаление(install,remove) -o- права доступа` |
| `DIR:` | **Создаёт/удаляет директорию** | `куда -o- установка/удаление(install,remove) -o- права доступа` |
| `SML:` | **Создаёт/удаляет ссылку** | `на что ссылается -o- где расположить -o- именовать/разъименовать(install,remove) -o- симлинк/жесткая(hardlink,symlink)` |
| `#P#` | **Выводит сообщение** | `сообщение` |
| `#` | **Комментарий** | `# Это коментарий` | 




#### Пример Файла
```targets
%Hello World
# выводит Hello World
#P# Hello World
# создаёт папку c всеобщим доступом
DIR:~/HelloWorld-o-install-o-777
# воспроизводит команду
$echo "Hello World"-o-critical
```
#### Пример использования в коде

```python
from NLUtils.Installer import NLInstaller

# Инициализация библиотеки
Inst = NLInstaller('install.targets')

# Запуск конкретной цели
Inst.RunTarget('Hello world')

```

## NLUtils.Logger

### `ConColors`

Класс статических переменных для цветовой разметки вывода в терминале (ANSI escape codes).

* **R, G, Y, B, V**: Цвета (Красный, Зелёный, Жёлтый, Синий, Фиолетовый).
* **S**: Сброс цвета (Reset).

### `NLLogger(production, ComponentName, logList)`

Логгер. Поддерживает вывод в консоль и/или аккумулирование логов в список.

* **Warning(warn)**: Выводит жёлтое сообщение о предупреждении. Не прерывает работу.
* **Error(err, critical)**: Выводит красное сообщение об ошибке. Если `critical=True`, вызывает `exit(1)`.
* **Info(inf, color, productionLatency)**: Выводит информационное сообщение заданного цвета. Параметр `productionLatency` управляет отображением в продакшн-режиме.

#### Пример использования в коде

```python
from NLUtils.Logger import NLLogger, ConColors

# Инициализация логгера для компонента
Logger = NLLogger(production=True, ComponentName='Core')

# Информационный вывод
Logger.Info("Starting system...", ConColors.G, True)

# Ошибка, которая не вешает систему
Logger.Error("Non-critical issue", critical=False)

```

## NLUtils.Translator

### `NLTranslator(production, language, WRITEMODE=False)`

Система локализации. Отвечает за сопоставление ключей интерфейса с их текстовыми значениями на разных языках.

* **language**: Если передано `language='Config'` а не например `language='RU'`, язык берется из файла настроек в корне проекта где дополнительно можно писать произвольные поля.

### Файл настроек `Settings.confJs`
```json
{
    "language": "RU"
}
```

### Методы

* **loadTranslation()**: Загружает `.ntrl` файл (JSON-структура) из директории `Translations`, используя путь к корню проекта.
* **Translate(entry)**: Возвращает переведенную строку по ключу `entry`.
* Если ключа нет и `writemode=False`: возвращает сам ключ и пишет ошибку в лог.
* Если ключа нет и `writemode=True`: записывает ключ в словарь и возвращает `'writen' ОСТОРОЖНО - перезаписывает существующие переводы`.



#### Формат файлов

* **Конфиг**: `Settings.confJs` (хранит текущий язык).
* **Словари**: `/Translations/[language].ntrl`.

#### Пример использования в коде

```python
from NLUtils.Translator import NLTranslator

# Инициализация (берем язык из настроек)
T = NLTranslator(production=True, language='Config', WRITEMODE=True)

# Обычный перевод
print(T.Translate('WelcomeMessage'))

# Если ключа 'ExitBtn' нет, он будет добавлен в словарь для последующего перевода
T.Translate('ExitBtn')

```
#### Пример языкового  перевода RU
```json
{
    "WelcomeMessage": "Добро пожаловать в Niritech Labs",
    "Error_UserNotFound": "Ошибка: Пользователь не найден в системе",
    "Install_Started": "Запуск процесса установки...",
    "btn_exit": "Выход"
}
```


## NLUtils.BlocksUtils

Фундаментальный модуль структуры данных. Определяет логику хранения, поиска и преобразования иерархических блоков.

### `Block(name)`

Базовый узел данных. Содержит в себе вложенные блоки и параметры.



| Метод | Описание |
| --- | --- |
| **FindParam(name)** | Ищет параметр только в текущем блоке. Возвращает `list[key, value]` или `None`. |
| **FindParamRecursive(name)** | Глубокий поиск параметра по всему дереву вниз от текущего узла. |
| **FindBlock(name)** | Ищет вложенный блок по имени на один уровень вниз. |
| **FindBlockRecursive(name)** | Рекурсивный поиск блока по всей глубине дерева. |
| **AddBlock(block)** | Регистрирует вложенный блок, автоматически обновляя его уровень вложенности (`level`). |
| **AddParam(key, value)** | Добавляет параметр в мутабельный список `params`. |
| **DeleteMarkedObjects()** | Очищает дерево от объектов, помеченных как `None` (используется для динамической чистки памяти). |
| **DeleteAllBlocks() / DeleteAllParams()** | Полная очистка содержимого узла. |
| **SetBlocks(list) / SetParams(list)** | Прямая перезапись списков содержимого. |
| **GetName()** | Возвращает имя блока. |
| **ToStr()** | Сериализует блок и его детей в строку формата `*.blocks` с использованием 8-пробельных отступов. |

---

## `Blocks(name)`

Наследник `Block`, выполняющий роль корня дерева и менеджера парсинга.

### Методы и Парсинг

* **FromStr(data) [Static]**: Превращает сырой текст в дерево объектов.
* Выбрасывает `ValueError`, если формат строки нарушен.


* **UpdateLevel()**: Принудительно синхронизирует уровни вложенности (`level`) для всех дочерних элементов (важно после ручного перемещения блоков).
* **AddNewRootBlock(name)**: "Обертка". Создает новый корень, перемещая текущее содержимое в дочерний блок.

#### Пример структуры в памяти

```python
# Cтруктура выглядит так:
block.params = [
    ['user', 'niris'],
    ['access', 'root']
]

```

#### Пример использования

```python
from NLUtils.BlocksUtils import Blocks

# Создание дерева из строки
Root = Blocks.FromStr(raw_text)

# Глубокий поиск
targetParams = Root.FindParamRecursive('home_dir')

# Модификация на лету
if targetParams:
    targetParams[0][1] = '/home/custom_path' 

# Превращение обратно в текст
print(Root.ToStr())

```

#### Пример файла
```blocks
name: NiritechSystem
    params:
        version = 2.0.50
        author = Niris
        environment = production

    blocks:
        name: User_Niris
            params:
                uid = 1001
                home = /home/niris
                shell = /bin/zsh

            blocks:
                name: Projects
                    params:
                        path = /home/niris/dev
                        auto_backup = true

        name: NetworkConfig
            params:
                hostname = nl-station-01
                ip = 192.168.1.50
```

Это завершающий кирпич в фундаменте твоих утилит. **ConfigManager** — это высокоуровневая обертка над стандартным `json`, которая берет на себя всю «грязную» работу с путями и правами доступа.

---

## NLUtils.JSONUtils

### `ConfigManager(path, production)`

Универсальный менеджер для работы с JSON-конфигурациями. Обеспечивает безопасное чтение и запись данных с автоматической нормализацией путей.

* **path**: Путь к основному файлу конфигурации.

### Методы

| Метод | Описание |
| --- | --- |
| **LoadConfig()** | Загружает основной конфиг. Если файл поврежден или отсутствует — создает пустой `{}` и сохраняет его, предотвращая падение системы. |
| **SaveConfig(dataToSave)** | Сохраняет словарь в основной файл. Если родительские директории не существуют, они будут созданы автоматически (`mkdir -p`). |
| **OpenRestricted(path)** | Позволяет прочитать любой произвольный JSON-файл. При ошибке возвращает `None` и логирует проблему через `NLLogger`. |
| **SaveRestricted(path, data)** | Записывает данные в указанный путь. Также поддерживает автоматическое создание структуры папок. |


#### Пример использования в коде

```python
from NLUtils.JSONUtils import ConfigManager

# Инициализация для конкретного конфига
CM = ConfigManager('~/Settings.confJs', production=True)

# Загрузка (безопасная)
settings = CM.LoadConfig()

# Изменение и сохранение
settings['last_run'] = '2026-03-12'
CM.SaveConfig(settings)

# Работа с внешним файлом (например, базой переводов)
data = CM.OpenRestricted('./Translations/RU.ntrl')

```
#EN


## NLUtils.Parser

### `ParserRealizationFabric(production, path, name)`

Abstract base class for all parser implementations.

* **open()**: Internal method for safely reading raw data from a file.
* **save()**: Internal method for safely writing raw data to a file.
* **Decode()**: Abstract method that must be overridden in descendants to convert text into `Blocks`.
* **Encode()**: An abstract method that must be overridden in descendants to convert `Blocks` to raw text.

### `NLParser(production)`

The central parsing manager. It stores a dictionary of available “realizations” and associates them with file types.

#### Methods

* **SetParserRealization(name, realizationClass)**: Registers a new parser type. Allows you to extend the system's capabilities without modifying the core.
* **OpenFile(path, type)**: Prepares the file for reading. Finds the required implementation by name `type`, creates an instance of it, and returns it.

#### Example of use in code

```python
from NLUtils.Parser import NLParser
from NLUtils.BaseParserRealizations import TargetsParser, BlocksParser

# Initialize the manager
Parser = NLParser(production=True)

# Register parser types
Parser.SetParserRealization(‘targets’, TargetsParser)
Parser.SetParserRealization(‘blocks’, BlocksParser)

# Read file (interface is always .Read())
# Returns an object of type Blocks
data = Parser.OpenFile(‘install.targets’, ‘targets’).Read()
# Now you can work with the data via the standard Blocks API.
print(data.GetName())

```


## NLUtils.BaseParserRealizations
### `TargetsParser(production, path)`
Parser implementation for linear `.targets` files. Uses prefixes.
### `BlocksParser:(production, path)`
Parser implementation for hierarchical `.blocks` files. Multi-format
### `HyprlangParser:(pathToTargets,root)`
Parser implementation for Hyprland files


## NLUtils.Installer
### `NLInstaller(production, path)`
* **RunTarget(name)**: Executes all instructions within the specified block `%name`.
* **RunCommand(command, type)**: Executes a system call. Types: `critical` (stop), `error`, `warning`.


#### Examples of `.targets` syntax

| Marker | Name | Value format (`-o-` separator) | 
| --- | --- | --- | 
| `%` | **Block name** | `%Name` |
| `$` | **Runs command** | `command -o- how to respond to error(critical,error,warning)` |
| `TRG:` | **Creates/deletes file** | `where -o- from where -o- install/remove(install,remove) -o- access rights` |
| `DIR:` | **Creates/deletes directory** | `where -o- install/remove(install,remove) -o- access rights` |
| `SML:` | **Creates/deletes a link** | `what it links to -o- where to place it -o- name/rename(install,remove) -o- symlink/hardlink(hardlink,symlink)` |
| `#P#` | **Outputs a message** | `message` |
| `#` | **Comment** | `# This is a comment` |




#### File Example


```targets
%Hello World
# outputs Hello World
#P# Hello World
# creates a folder with universal access
DIR:~/HelloWorld-o-install-o-777
# reproduces the command
$echo “Hello World”-o-critical
```
#### Example of use in code

```python
from NLUtils.Installer import NLInstaller

# Initialize the library
Inst = NLInstaller(‘install.targets’)

# Run a specific target
Inst.RunTarget(‘Hello world’)

```

## NLUtils.Logger

### `ConColors`

A class of static variables for color marking output in the terminal (ANSI escape codes).

* **R, G, Y, B, V**: Colors (Red, Green, Yellow, Blue, Violet).
* **S**: Color reset.

### `NLLogger(production, ComponentName, logList)`

Logger. Supports console output and/or accumulation of logs in a list.

* **Warning(warn)**: Outputs a yellow warning message. Does not interrupt operation.
* **Error(err, critical)**: Outputs a red error message. If `critical=True`, calls `exit(1)`.
* **Info(inf, color, productionLatency)**: Outputs an informational message of the specified color. The `productionLatency` parameter controls display in production mode.

#### Example of use in code

```python
from NLUtils.Logger import NLLogger, ConColors

# Initialize the logger for the component
Logger = NLLogger(production=True, ComponentName=‘Core’)

# Informational output
Logger.Info(“Starting system...”, ConColors.G, True)

# Error that does not hang the system
Logger.Error(“Non-critical issue”, critical=False)

```

## NLUtils.Translator

### `NLTranslator(production, language, WRITEMODE=False)`

Localization system. Responsible for mapping interface keys to their text values in different languages.

* **language**: If `language=‘Config’` is passed instead of, for example, `language=‘RU’`, the language is taken from the settings file in the project root, where you can additionally write arbitrary fields.

### Settings file `Settings.confJs`
```json
{
    “language”: “EN”
}
```

### Methods

* **loadTranslation()**: Loads the `.ntrl` file (JSON structure) from the `Translations` directory using the path to the project root.
* **Translate(entry)**: Returns the translated string by the `entry` key.
* If the key does not exist and `writemode=False`: returns the key itself and writes an error to the log.
* If the key does not exist and `writemode=True`: writes the key to the dictionary and returns `‘writen’ CAUTION - overwrites existing translations`.



#### File format

* **Config**: `Settings.confJs` (stores the current language).
* **Dictionaries**: `/Translations/[language].ntrl`.

#### Example of use in code

```python
from NLUtils.Translator import NLTranslator

# Initialization (take language from settings)
T = NLTranslator(production=True, language=‘Config’, WRITEMODE=True)

# Regular translation
print(T.Translate(‘WelcomeMessage’))

# If the ‘ExitBtn’ key does not exist, it will be added to the dictionary for subsequent translation
T.Translate(‘ExitBtn’)

```
#### Example of language  translation EN

```json
{
    “WelcomeMessage”: “Welcome to Niritech Labs”,
    “Error_UserNotFound”: “Error: User not found in the system”,
    “Install_Started”: “Starting installation process...”,
    “btn_exit”: “Exit”
}
```


## NLUtils.BlocksUtils

Fundamental data structure module. Defines the logic for storing, searching, and converting hierarchical blocks.

### `Block(name)`

Basic data node. Contains nested blocks and parameters.



| Method | Description |
| --- | --- |
| **FindParam(name)** | Searches for a parameter only in the current block. Returns `list[key, value]` or `None`. |
| **FindParamRecursive(name)** | Deep search for a parameter throughout the tree down from the current node. |
| **FindBlock(name)** | Searches for a nested block by name one level down. |
| **FindBlockRecursive(name)** | Recursive search for a block throughout the entire depth of the tree. |
| **AddBlock(block)** | Registers a nested block, automatically updating its nesting level (`level`). |
| **AddParam(key, value)** | Adds a parameter to the mutable list `params`. |
| **DeleteMarkedObjects()** | Clears the tree of objects marked as `None` (used for dynamic memory cleanup). |
| **DeleteAllBlocks() / DeleteAllParams()** | Completely clears the contents of a node. |
| **SetBlocks(list) / SetParams(list)** | Directly overwrites the content lists. |
| **GetName()** | Returns the name of the block. |
| **ToStr()** | Serializes the block and its children into a string of the format `*.blocks` using 8-space indents. |

---


## `Blocks(name)`

A descendant of `Block` that acts as the root of the tree and the parsing manager.

### Methods and Parsing

* **FromStr(data) [Static]**: Converts raw text into a tree of objects.
* Raises `ValueError` if the string format is broken.


* **UpdateLevel()**: Forcibly synchronizes nesting levels (`level`) for all child elements (important after manually moving blocks).
* **AddNewRootBlock(name)**: “Wrapper.” Creates a new root, moving the current content to a child block.

#### Example of structure in memory

```python
# The structure looks like this:
block.params = [
    [‘user’, ‘niris’],
    [‘access’, ‘root’]
]

```

#### Example of use

```python
from NLUtils.BlocksUtils import Blocks

# Creating a tree from a string
Root = Blocks.FromStr(raw_text)

# Deep search
targetParams = Root.FindParamRecursive(‘home_dir’)

# Modification on the fly
if targetParams:
    targetParams[0][1] = ‘/home/custom_path’ 

# Convert back to text
print(Root.ToStr())

```

#### File example
```blocks
name: NiritechSystem
    params:
        version = 2.0.50
        author = Niris
        environment = production

    blocks:
        name: User_Niris
            params:
                uid = 1001
                home = /home/niris
                shell = /bin/zsh

            blocks:
                name: Projects
                    params:
                        path = /home/niris/dev
                        auto_backup = true

        name: NetworkConfig
            params:
                hostname = nl-station-01
                ip = 192.168.1.50
```

This is the final brick in the foundation of your utilities. **ConfigManager** is a high-level wrapper over standard `json` that takes care of all the “dirty” work with paths and access rights.

---

## NLUtils.JSONUtils

### `ConfigManager(path, production)`

A universal manager for working with JSON configurations. Provides secure reading and writing of data with automatic path normalization.

* **path**: The path to the main configuration file.

### Methods

| Method | Description |
| --- | --- |
| **LoadConfig()** | Loads the main config. If the file is corrupted or missing, creates an empty `{}` and saves it, preventing the system from crashing. |
| **SaveConfig(dataToSave)** | Saves the dictionary to the main file. If the parent directories do not exist, they will be created automatically (`mkdir -p`). |
| **OpenRestricted(path)** | Allows you to read any arbitrary JSON file. If an error occurs, it returns `None` and logs the problem via `NLLogger`. |
| **SaveRestricted(path, data)** | Writes data to the specified path. Also supports automatic folder structure creation. |


#### Example of use in code

```python
from NLUtils.JSONUtils import ConfigManager

# Initialization for a specific config
CM = ConfigManager(‘~/Settings.confJs’, production=True)

# Loading (secure)
settings = CM.LoadConfig()

# Modification and saving
settings[‘last_run’] = ‘2026-03-12’
CM.SaveConfig(settings)

# Working with an external file (e.g., translation database)
data = CM.OpenRestricted(‘./Translations/EN.ntrl’)

```






