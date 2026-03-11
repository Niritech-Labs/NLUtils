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


