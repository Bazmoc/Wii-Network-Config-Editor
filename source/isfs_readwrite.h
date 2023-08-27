/*
Those functions are used for reading and writing to the NAND(isfs)
*/

static fstats stats ATTRIBUTE_ALIGN(32);

u8 *ISFS_GetFile(u8 *path, u32 *size, s32 length){
	
	*size = 0;
	
	s32 fd = ISFS_Open((const char *)path, ISFS_OPEN_READ);
		u8 *buf = NULL;

		if(fd >= 0)
		{
			memset(&stats, 0, sizeof(fstats));
			if(ISFS_GetFileStats(fd, &stats) >= 0)
			{
				if(length <= 0)
					length = stats.file_length;
				if(length > 0)
					buf = (u8 *)memalign(32, length);
				if(buf)
				{
					*size = stats.file_length;
					if(ISFS_Read(fd, (char*)buf, length) != length){
						*size = 0;
						free(buf);
					}
				}
			}
		ISFS_Close(fd);
		}

		if(*size > 0)
		{	
			DCFlushRange(buf, *size);
			ICInvalidateRange(buf, *size);
		}
		return buf;
}

u8 *ISFS_WRITE_CONFIGDAT(u8 *buf2){ 
	s32 fd2 = ISFS_Open(ISFS_CONFIGDAT_PATH, ISFS_OPEN_WRITE); //opens config.dat on the nand. Write mode
	if(fd2<0) //negative = error so we show what error it is
	{
		//printf("Failed to open, error %d\n", fd2);
		exit(0);
	}
	else //everything's fine:
	{
		//printf("opened isfs \n");
		s32 ret = ISFS_Write(fd2, buf2, 7004);
		if(ret<0){ //like before, under 0 is an error code so we show it.
			//printf("Error %d \n", ret);
					exit(0);
		}
		//printf("isfs write ok \n");
		/*free(buf2);
		printf("buf2 freed \n");
		*/
		ISFS_Close(fd2);
		//printf("ISFS closed \n");
	}
	return 0;
}
