//this function takes the length of the new SSID and the password and returns a hex value which needs to be written in the file next to the password and SSID.
	
char lengthtoHex(int text_length){ 
	switch(text_length){
		case 0:
			return 0x00;
			break;
		case 1:
			return 0x01;
			break;
		case 2:
			return 0x02;
			break;
		case 3:
			return 0x03;
			break;
		case 4:
			return 0x04;
			break;
		case 5:
			return 0x05;
			break;
		case 6:
			return 0x06;
			break;
		case 7:
			return 0x07;
			break;
		case 8:
			return 0x08;
			break;
		case 9:
			return 0x09;
			break;
		case 10:
			return 0x0A;
			break;
		case 11:
			return 0x0B;
			break;
		case 12:
			return 0x0C;
			break;
		case 13:
			return 0x0D;
			break;
		case 14:
			return 0x0E;
			break;
		case 15:
			return 0x0F;
			break;
		case 16:
			return 0x10;
			break;
		case 17:
			return 0x11;
			break;
		case 18:
			return 0x12;
			break;
		case 19:
			return 0x13;
			break;
		case 20:
			return 0x14;
			break;
		case 21:
			return 0x15;
			break;
		case 22:
			return 0x16;
			break;
		case 23:
			return 0x17;
			break;
		case 24:
			return 0x18;
			break;
		case 25:
			return 0x19;
			break;
		case 26:
			return 0x1A;
			break;
		case 27:
			return 0x1B;
			break;
		case 28:
			return 0x1C;
			break;
		case 29:
			return 0x1D;
			break;
		case 30:
			return 0x1E;
			break;
		case 31:
			return 0x1F;
			break;
		case 32:
			return 0x20;
			break;
		case 33:
			return 0x21;
			break;
		case 34:
			return 0x22;
			break;
		case 35:
			return 0x23;
			break;
		case 36:
			return 0x24;
			break;
		case 37:
			return 0x25;
			break;
		case 38:
			return 0x26;
			break;
		case 39:
			return 0x27;
			break;
		case 40:
			return 0x28;
			break;
		case 41:
			return 0x29;
			break;
		case 42:
			return 0x2A;
			break;
		case 43:
			return 0x2B;
			break;
		case 44:
			return 0x2C;
			break;
		case 45:
			return 0x2D;
			break;
		case 46:
			return 0x2E;
			break;
		case 47:
			return 0x2F;
			break;
		case 48:
			return 0x30;
			break;
		case 49:
			return 0x31;
			break;
		case 50:
			return 0x32;
			break;
		case 51:
			return 0x33;
			break;
		case 52:
			return 0x34;
			break;
		case 53:
			return 0x35;
			break;
		case 54:
			return 0x36;
			break;
		case 55:
			return 0x37;
			break;
		case 56:
			return 0x38;
			break;
		case 57:
			return 0x39;
			break;
		case 58:
			return 0x3A;
			break;
		case 59:
			return 0x3B;
			break;
		case 60:
			return 0x3C;
			break;
		case 61:
			return 0x3D;
			break;
		case 62:
			return 0x3E;
			break;
		case 63:
			return 0x3F;
			break;
		default:   //means some kind of bug happened because the number must be between 0 and 63, so if it's not any of the numbers we just return 0x00
			return 0x00;
			break;
	}
}