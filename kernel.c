void kernel_main() {
    char* video = (char*)0xB8000;
    video[0] = 'M';
    video[1] = 0x07;
    video[2] = 'y';
    video[3] = 0x07;
    video[4] = 'O';
    video[5] = 0x07;
    video[6] = 's';
    video[7] = 0x07;
}