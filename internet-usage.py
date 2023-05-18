"""
Используя dbus, отслеживает изменения в сетевом интерфейсе и обновляет данные по использованию интернета
type='signal', interface='org.freedesktop.NetworkManager'
Записывает файл когда у нас есть соединение  и когда его нет
"""
from pydbus import SystemBus
from gi.repository import GLib
import datetime

# Путь к файлу где хранится информация об использовании интернета.
USAGE_FILE = "/home/inex/dev/Odelchi/internet-usage/file.txt"

session_bus = SystemBus()
loop = GLib.MainLoop()

def handle_state_change(state):
    if state == 70:
        # Системный интерфейс подключен.
        # Запись актуального времени в файл.
        with open(USAGE_FILE, "a") as usage_file:
            usage_file.write(f"{datetime.datetime.now()} СОЕДИНЕНО\n")

    elif state == 50:
        # Системный интерфейс не подключен.
        # Записывает время в файл.
        with open(USAGE_FILE, "a") as usage_file:
            usage_file.write(f"{datetime.datetime.now()} НЕСОЕДИНЕНО\n")


nm = session_bus.get("org.freedesktop.NetworkManager")

nm.StateChanged.connect(handle_state_change)

loop.run()
