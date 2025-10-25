package main

import (
	"fmt"
	"os"
	"strings"
	"time"
)

const (
	blockSize = 256
	maxFiles  = 64
	maxPtrs   = 8
	maxName   = 56
	maxUser   = 40
)

type FNEntry struct {
	Name  [maxName]byte
	Inode int32
}

type DAPEntry struct {
	Size int32
	Time int64
	User [maxUser]byte
	Ptrs [maxPtrs]int32
}

var (
	disk        []byte
	fnt         [maxFiles]FNEntry
	dabpt       [maxFiles]DAPEntry
	currentUser = "root"
)

func toBytes(s string, b []byte) {
	copy(b, []byte(s))
}

func findFile(name string) int {
	for i, e := range fnt {
		if strings.Trim(string(e.Name[:]), "\x00") == name {
			return i
		}
	}
	return -1
}

func allocateInode() int {
	for i := range dabpt {
		if dabpt[i].Size == 0 {
			return i
		}
	}
	return -1
}

func allocateBlocks() int32 {
	for i := int32(0); i < int32(len(disk)/blockSize); i++ {
		if disk[i*blockSize] == 0 {
			return i
		}
	}
	return -1
}

func Createfs(n int) {
	disk = make([]byte, n*blockSize)
	fmt.Println("FS created")
}

func Formatfs() {
	fnt = [maxFiles]FNEntry{}
	dabpt = [maxFiles]DAPEntry{}
	fmt.Println("FS formatted")
}

func Savefs(name string) {
	os.WriteFile(name, disk, 0644)
	fmt.Println("Saved", name)
}

func Openfs(name string) {
	data, _ := os.ReadFile(name)
	disk = data
	fmt.Println("Opened", name)
}

func List() {
	fmt.Println("files:")
	for _, e := range fnt {
		n := strings.Trim(string(e.Name[:]), "\x00")
		if n != "" {
			in := dabpt[e.Inode]
			fmt.Printf("%s (%d bytes) owner:%s\n", n, in.Size,
				strings.Trim(string(in.User[:]), "\x00"))
		}
	}
}

func Usercmd(name string) {
	currentUser = name
	fmt.Println("current user:", name)
}

func Put(ext string, fsname string) {
	data, _ := os.ReadFile(ext)
	idx := findFile(fsname)
	if idx != -1 {
		fmt.Println("file exists")
		return
	}
	in := allocateInode()
	if in == -1 {
		fmt.Println("no inodes")
		return
	}
	b := allocateBlocks()
	if b == -1 {
		fmt.Println("disk full")
		return
	}
	copy(disk[b*blockSize:], data)
	toBytes(fsname, fnt[in].Name[:])
	fnt[in].Inode = int32(in)

	dabpt[in].Size = int32(len(data))
	dabpt[in].Time = time.Now().Unix()
	toBytes(currentUser, dabpt[in].User[:])
	dabpt[in].Ptrs[0] = b
	fmt.Println("stored", fsname)
}

func Get(fsname, ext string) {
	i := findFile(fsname)
	if i < 0 {
		fmt.Println("no file")
		return
	}
	in := dabpt[fnt[i].Inode]
	b := in.Ptrs[0]
	data := disk[b*blockSize : b*blockSize+in.Size]
	os.WriteFile(ext, data, 0644)
	fmt.Println("Copied to", ext)
}

func Remove(name string) {
	i := findFile(name)
	if i < 0 {
		fmt.Println("not found")
		return
	}
	dabpt[fnt[i].Inode] = DAPEntry{}
	fnt[i] = FNEntry{}
	fmt.Println("deleted", name)
}

func Rename(oldn, newn string) {
	i := findFile(oldn)
	if i < 0 {
		fmt.Println("not found")
		return
	}
	toBytes(newn, fnt[i].Name[:])
	fmt.Println("renamed", oldn, "to", newn)
}

func main() {
	for {
		var cmd, a, b string
		fmt.Print("> ")
		fmt.Scan(&cmd, &a, &b)
		switch cmd {
		case "Createfs":
			n := 0
			fmt.Sscan(a, &n)
			Createfs(n)
		case "Formatfs":
			Formatfs()
		case "Savefs":
			Savefs(a)
		case "Openfs":
			Openfs(a)
		case "List":
			List()
		case "Remove":
			Remove(a)
		case "Rename":
			Rename(a, b)
		case "Put":
			Put(a, b)
		case "Get":
			Get(a, b)
		case "User":
			Usercmd(a)
		default:
			fmt.Println("n/a")
		}
	}
}
