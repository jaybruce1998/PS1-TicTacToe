#include <stdio.h>
#include "ps1/gpucmd.h"
#include "ps1/registers.h"
#include "controller.h"
#include "gpu.h"
#include "font.h"

int RAINBOW[3][3]={{255, 0, 0},
				   {0, 255, 0},
				   {0, 0, 255}};

int GREEN[3][3]={{0, 255, 0},
				 {0, 255, 0},
				 {0, 255, 0}};

char board[3][3];
char moved=0, movedBefore=0, winner, turn, turns;
int curRow=0, curCol=0;

extern const uint8_t fontTexture[], fontPalette[];

static void drawRectangle(int x1, int y1, int x2, int y2, int colors[3][3]) {
    GPU_GP0 = gp0_rgb(colors[0][0], colors[0][1], colors[0][2]) | gp0_shadedTriangle(true, false, false); // Red triangle
    GPU_GP0 = gp0_xy(x1, y1);                             // Top-left vertex
    GPU_GP0 = gp0_rgb(colors[1][0], colors[1][1], colors[1][2]);
    GPU_GP0 = gp0_xy(x2, y1);                             // Top-right vertex
    GPU_GP0 = gp0_rgb(colors[2][0], colors[2][1], colors[2][2]);
    GPU_GP0 = gp0_xy(x1, y2);                             // Bottom-left vertex

    GPU_GP0 = gp0_rgb(colors[2][0], colors[2][1], colors[2][2]) | gp0_shadedTriangle(true, false, false); // Blue triangle
    GPU_GP0 = gp0_xy(x2, y1);                             // Top-right vertex
    GPU_GP0 = gp0_rgb(colors[1][0], colors[1][1], colors[1][2]);
    GPU_GP0 = gp0_xy(x2, y2);                             // Bottom-right vertex
    GPU_GP0 = gp0_rgb(colors[0][0], colors[0][1], colors[0][2]);
    GPU_GP0 = gp0_xy(x1, y2);                             // Bottom-left vertex
}

static void drawX(int x1, int y1, int x2, int y2, int m) {
    // Calculate the midpoint of the rectangle
    int midX = (x1 + x2) / 2;
    int midY = (y1 + y2) / 2;

    // Draw the first triangle (top-left to bottom-right)
    GPU_GP0 = gp0_rgb(0, 0, 255) | gp0_shadedTriangle(true, false, false);
    GPU_GP0 = gp0_xy(x1, y1);                       // Top-left vertex
    GPU_GP0 = gp0_rgb(0, 0, 255);                   // Green vertex
    GPU_GP0 = gp0_xy(x2, y2);                       // Bottom-right vertex
    GPU_GP0 = gp0_rgb(0, 0, 255);                   // Blue vertex
    GPU_GP0 = gp0_xy(x1, y2);                       // Bottom-left vertex

    // Draw the second triangle (top-right to bottom-left)
    GPU_GP0 = gp0_rgb(0, 0, 255) | gp0_shadedTriangle(true, false, false);
    GPU_GP0 = gp0_xy(x2, y1);                       // Top-right vertex
    GPU_GP0 = gp0_rgb(0, 0, 255);                   // Blue vertex
    GPU_GP0 = gp0_xy(x1, y2);                       // Bottom-left vertex
    GPU_GP0 = gp0_rgb(0, 0, 255);                   // Blue vertex
    GPU_GP0 = gp0_xy(x2, y2);                       // Bottom-right vertex

    // Bottom gray triangle
    GPU_GP0 = gp0_rgb(64, 64, 64) | gp0_shadedTriangle(true, false, false);
    GPU_GP0 = gp0_xy(x1+m, y2);
    GPU_GP0 = gp0_rgb(64, 64, 64);
    GPU_GP0 = gp0_xy(midX, midY+m);
    GPU_GP0 = gp0_rgb(64, 64, 64);
    GPU_GP0 = gp0_xy(x2-m, y2);

    // Left gray triangle
    GPU_GP0 = gp0_rgb(64, 64, 64) | gp0_shadedTriangle(true, false, false);
    GPU_GP0 = gp0_xy(x1, y1+m);
    GPU_GP0 = gp0_rgb(64, 64, 64);
    GPU_GP0 = gp0_xy(midX-m, midY);
    GPU_GP0 = gp0_rgb(64, 64, 64);
    GPU_GP0 = gp0_xy(x1, y2-m);

    // Right gray triangle
    GPU_GP0 = gp0_rgb(64, 64, 64) | gp0_shadedTriangle(true, false, false);
    GPU_GP0 = gp0_xy(x2, y1+m);
    GPU_GP0 = gp0_rgb(64, 64, 64);
    GPU_GP0 = gp0_xy(midX+m, midY);
    GPU_GP0 = gp0_rgb(64, 64, 64);
    GPU_GP0 = gp0_xy(x2, y2-m);
}

static void drawTriangle(int x1, int triWidth, int y1, int triHeight) {
	int y2=y1+triHeight;
    GPU_GP0 = gp0_rgb(255, 0, 0) | gp0_shadedTriangle(true, false, false); // Red triangle
    GPU_GP0 = gp0_xy(x1, y2);
    GPU_GP0 = gp0_rgb(255, 0, 0);
    GPU_GP0 = gp0_xy(x1+triWidth, y1);
    GPU_GP0 = gp0_rgb(255, 0, 0);
    GPU_GP0 = gp0_xy(x1+triWidth*2, y2);
}

static void drawCursor(int x1, int y1, int x2, int y2) {
	drawRectangle(x1, y1, x2, y1+5, GREEN);
	drawRectangle(x2-5, y1, x2, y2, GREEN);
	drawRectangle(x1, y2-5, x2, y2, GREEN);
	drawRectangle(x1, y1, x1+5, y2, GREEN);
}

static void drawGrid(int rectWidth, int rectHeight, int triWidth, int triHeight) {
	drawRectangle(0, rectHeight, rectWidth*4, rectHeight+triHeight, RAINBOW);
	drawRectangle(0, rectHeight*2+triHeight, rectWidth*4, rectHeight*3, RAINBOW);
	drawRectangle(rectWidth, 0, rectWidth+triWidth, rectHeight*4, RAINBOW);
	drawRectangle(rectWidth*2+triWidth, 0, rectWidth*3, rectHeight*4, RAINBOW);
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			if(board[i][j]=='X')
				drawX(j*triWidth*3, i*triHeight*3, j*triWidth*3+rectWidth, i*triHeight*3+rectHeight, 5);
			else if(board[i][j]=='O')
				drawTriangle(j*triWidth*3, triWidth, i==0?8:i*triHeight*3, i==0?rectHeight-8:rectHeight);
	drawCursor(curCol*triWidth*3, curRow==0?8:curRow*triHeight*3, rectWidth+curCol*triWidth*3, rectHeight+curRow*triHeight*3);
}

static void resetBoard() {
	winner=0;
	turn='X';
	turns=0;
	for(int i=0; i<3; i++)
		for(int j=0; j<3; j++)
			board[i][j]=' ';
}

static void processInput(int port) {
	// Build the request packet.
	uint8_t request[4], response[8];

	request[0] = CMD_POLL; // Command
	request[1] = 0x00;     // Multitap address
	request[2] = 0x00;     // Rumble motor control 1
	request[3] = 0x00;     // Rumble motor control 2

	// Send the request to the specified controller port and grab the response.
	// Note that this is a relatively slow process and should be done only once
	// per frame, unless higher polling rates are desired.
	selectPort(port);
	int respLength = exchangePacket(
		ADDR_CONTROLLER, request, response, sizeof(request), sizeof(response)
	);

	if (respLength < 4) {
		// All controllers reply with at least 4 bytes of data.
		return;
	}

	// Bytes 2 and 3 hold a bitfield representing the state all buttons. As each
	// bit is active low (i.e. a zero represents a button being pressed), the
	// entire field must be inverted.
	uint16_t buttons = (response[2] | (response[3] << 8)) ^ 0xffff;

	//Start
	if ((buttons >> 3) & 1 && winner)
		resetBoard();
	moved=0;
	//Up/Down
	if((buttons >> 4) & 1) {
		moved=1;
		if(!movedBefore)
			curRow=curRow==0?2:curRow-1;
	}
	else if((buttons >> 6) & 1) {
		moved=1;
		if(!movedBefore)
			curRow=curRow==2?0:curRow+1;
	}
	//Right/Left
	if((buttons >> 5) & 1) {
		moved=1;
		if(!movedBefore)
			curCol=curCol==2?0:curCol+1;
	}
	else if((buttons >> 7) & 1) {
		moved=1;
		if(!movedBefore)
			curCol=curCol==0?2:curCol-1;
	}
	//X button
	if((buttons >> 14) & 1 && !winner && board[curRow][curCol]==' ')
	{
		board[curRow][curCol]=turn;
		if((board[0][curCol]==turn&&board[1][curCol]==turn&&board[2][curCol]==turn)||(board[curRow][0]==turn&&board[curRow][1]==turn&&board[curRow][2]==turn)||(board[1][1]==turn&&
		   ((board[0][0]==turn&&board[2][2]==turn)||(board[0][2]==turn&&board[2][0]==turn))))
			winner=turn;
		else if(++turns==9)
			winner='T';
		else
			turn=turn=='X'?'O':'X';
	}
	movedBefore=moved;
}

int main(int argc, const char **argv) {
	initSerialIO(115200);
	initControllerBus();

	if ((GPU_GP1 & GP1_STAT_MODE_BITMASK) == GP1_STAT_MODE_PAL) {
		setupGPU(GP1_MODE_PAL, SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		setupGPU(GP1_MODE_NTSC, SCREEN_WIDTH, SCREEN_HEIGHT);
	}

	DMA_DPCR |= DMA_DPCR_ENABLE << (DMA_GPU * 4);

	waitForGP0Ready();
	GPU_GP0 = gp0_texpage(0, true, false);
	GPU_GP0 = gp0_fbOffset1(0, 0);
	GPU_GP0 = gp0_fbOffset2(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1);
	GPU_GP0 = gp0_fbOrigin(0, 0);

	// Fill background with solid gray
	waitForGP0Ready();
	GPU_GP0 = gp0_rgb(64, 64, 64) | gp0_vramFill();
	GPU_GP0 = gp0_xy(0, 0);
	GPU_GP0 = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

	int gridCols = 5, gridRows = 5;
	int rectWidth = SCREEN_WIDTH / gridCols;
	int rectHeight = SCREEN_HEIGHT / gridRows;
	int triWidth = rectWidth / 2;
	int triHeight = rectHeight / 2;

	// Display the output
	GPU_GP1 = gp1_fbOffset(0, 0);
	GPU_GP1 = gp1_dmaRequestMode(GP1_DREQ_GP0_WRITE);
	GPU_GP1 = gp1_dispBlank(false);

	TextureInfo font;

	uploadIndexedTexture(
		&font, fontTexture, fontPalette, SCREEN_WIDTH * 2, 0, SCREEN_WIDTH * 2,
		FONT_HEIGHT, FONT_WIDTH, FONT_HEIGHT, FONT_COLOR_DEPTH
	);

	DMAChain dmaChains[2];
	bool     usingSecondFrame = false;

	resetBoard();
	for (;;) {
		int bufferX = usingSecondFrame ? SCREEN_WIDTH : 0;
		int bufferY = 0;

		DMAChain *chain  = &dmaChains[usingSecondFrame];
		usingSecondFrame = !usingSecondFrame;

		uint32_t *ptr;

		GPU_GP1 = gp1_fbOffset(bufferX, bufferY);

		chain->nextPacket = chain->data;

		ptr    = allocatePacket(chain, 4);
		ptr[0] = gp0_texpage(0, true, false);
		ptr[1] = gp0_fbOffset1(bufferX, bufferY);
		ptr[2] = gp0_fbOffset2(bufferX + SCREEN_WIDTH - 1, bufferY + SCREEN_HEIGHT - 2);
		ptr[3] = gp0_fbOrigin(bufferX, bufferY);

		ptr    = allocatePacket(chain, 3);
		ptr[0] = gp0_rgb(64, 64, 64) | gp0_vramFill();
		ptr[1] = gp0_xy(bufferX, bufferY);
		ptr[2] = gp0_xy(SCREEN_WIDTH, SCREEN_HEIGHT);

		drawGrid(rectWidth, rectHeight, triWidth, triHeight);
		processInput(0);
		switch(winner)
		{
		case 'X':
			printString(chain, &font, 16, 200, "X wins!");
			break;
		case 'O':
			printString(chain, &font, 16, 200, "Triangle wins!");
			break;
		case 'T':
			printString(chain, &font, 16, 200, "Tie!");
		}
		*(chain->nextPacket) = gp0_endTag(0);

		waitForGP0Ready();
		waitForVSync();
		sendLinkedList(chain->data);
	}

	return 0;
}
