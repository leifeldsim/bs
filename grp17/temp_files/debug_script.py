import pyautogui
from time import sleep

sleep(2)
commands = ["gdb osmprun", "set follow-fork-mode child", "set detach-on-fork off", "b main", "run 2 echoall test"]

for i in range(len(commands)):

    pyautogui.typewrite(commands[i])
    pyautogui.press("Enter")
    sleep(0.3)
