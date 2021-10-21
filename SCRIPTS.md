# BKEmu functions in Lua scripts

## Short description

- **LoadBin(strFileName)** - Load Binary file strFileName
- **LoadBreakpoints(strFileName)** - Load breakpoints from strFileName
- **LoadSymbolTable(strFileName)** - Load symbol table


- **GetImageName(nFddNumber)** - Return path to the image mounted to FDD
- **MountImage(nFddNumber, strImageName)** - Mount strImageName to FDD
- **UnMountImage(nFddNumber)** - Unmount image from Fdd


- **PressKey(nKey)** - Send Keycode to BK
- **SendStringToBk(strText, nDelay)** - Send characters from strText to BK one-by-one. Switch to Rus/Lat if need. nDelay - delay between chars in mSec. If omitted, used 80 mSec delay


- **poke(nAddr, nData)** - Store word nData into nAddr
- **pokeb(nAddr, bData)** - Store byte bData into nAddr
- **peek(nAddr)** - Return word in nAddr
- **peekb(nAddr)** - Return byte in nAddr

- **sleep(mSec)**             - Sleep mSec millicesonds
- **StopCPU()**                - Stop CPU if running
- **StartCPU()**               - Start CPU if not running

## Full description

#### LoadBin(strFileName), LoadBreakpoints(strFileName), LoadSymbolTable(strFileName)
Load Binary, Breakpoints or symbol table from file 'strFileName'
- **args**:
strFileName - string
- **return**:
true  if succes
false if fail

#### GetImageName(nFddNumber)
Return path to the image mounted to FDD
- **args**:
nFddNumber - number
- **return**:
 ImageName or "" if empty - string

#### UnMountImage(nFddNumber)
Unmount image from Fdd
- **args**:
nFddNumber - number
- **return**:
true  if succes
false if fail

#### MountImage(nFddNumber, strImageName)
Mount strImageName to FDD
- **args**:
nFddNumber - number
strImageName - string
- **return**:
true  if succes
false if fail


#### PressKey(nKey)
Send Keycode to BK. PressKey(0) will emulate press of 'STOP' button
- **args**:
nKey - number


#### SendStringToBk(strText, nDelay)
Send characters from strText to BK one-by-one. Switch to Rus/Lat if need. nDelay - delay between chars in mSek. If omitted, used 80 mSec delay
- **args**:
strText - string
nDelay - number


#### poke(nAddr, nData),  pokeb(nAddr, bData)
Store word or byte nData into nAddr
- **args**:
nAddr - number
nData - number


#### peek(nAddr), peekb(nAddr)
Return word or byte from nAddr
- **args**:
nAddr - number
- **return**:
number


#### sleep(mSec)
Sleep mSec millicesonds
- **args**:
mSec - number

#### StopCPU()
Stop CPU if running

#### StartCPU()
Start CPU if not running

### Examples

```lua
-- Sending multilanguage string to BK Rus/Lat
SendStringToBK("qwe йцу asd\n")

-- Simple programm
local t = {}
t[#t+1] = "10 BEEP"
t[#t+1] = "20 GOTO 10"
t[#t+1] = "RUN"
t[#t+1] = ""
a = table.concat(t, "\n")
SendStringToBK(a, 70)

-- Sleep 2 seconds
Sleep(2000)
-- Pause CPU
StopCPU()
-- Sleep 5 seconds
Sleep(5000)
-- Start CPU
StartCPU()

-- Send 'STOP' key
PressKey(0)

```

#### Fill Screen
```lua
local p = Poke
for i=1,10000 do
  p(040000 + i, i)
end

```
#### Build project and copy resulting BIN to the mounted disk image
```lua
imgName = GetImageName(0)
if imgName ~= "" then
    -- Copy file to image by external utility
    if os.execute("cd workdir && make") then 
        -- Unmount disk
        UnMountImage(0)
        -- Copy bin to the disk image
        os.execute("bkimg -c game.bin "  .. imgName)
        -- Mount the disk image back
        MountImage(0, imgName)
	end
end
```

## Notice for developers

You can easily add any new Lua function in emutools/LuaScripts.[h,cpp]. However there are some restrictions you want to follow:

1.  Because any updates to UI can be done in the main application thread only, any function changing UI **must** be called using emit signal
2. Because we can stop Lua script execution inside Lua hook function only, all C-implemented Lua functions **must** ether be fast enough or **must** check periodically status of (*bool*)***StopScriptThread*** variable and exit immediately if ***StopScriptThread == true***


