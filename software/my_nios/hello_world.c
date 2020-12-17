/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/alt_timestamp.h>
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"
#include "sys/alt_irq.h"
#include "system.h"

#define PIO_INTERRUPT_BASE 0x83020
#define PIO_INTERRUPT_IRQ_INTERRUPT_CONTROLLER_ID 0
#define PIO_INTERRUPT_IRQ 2

#define MATCHES_NUMBER 500
#define MATCHES_MEM_SIZE 1000


typedef struct Points
{
	uint16_t x;
	uint16_t y;
}Point;

/* Declare a global variable to hold the edge capture value. */
volatile int edge_capture;

static void handle_button_interrupts(void* context)
{
	/* Cast context to edge_capture's type. It is important that this
	be declared volatile to avoid unwanted compiler optimization. */
	volatile int* edge_capture_ptr = (volatile int*) context;
	/*
	 * Read the edge capture register on the button PIO.
	 * Store value.
	 */
	*edge_capture_ptr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(PIO_INTERRUPT_BASE);
	/* Write to the edge capture register to reset it. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_INTERRUPT_BASE, 0);
	/* Read the PIO to delay ISR exit. This is done to prevent a
	spurious interrupt in systems with high processor -> pio
	latency and fast interrupts. */
	IORD_ALTERA_AVALON_PIO_EDGE_CAP(PIO_INTERRUPT_BASE);
}

static void init_button_pio()
{
	/* Recast the edge_capture pointer to match the
	alt_irq_register() function prototype. */
	void* edge_capture_ptr = (void*) &edge_capture;
	/* Enable 1 button interrupts. */
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(PIO_INTERRUPT_BASE, 0x1);
	/* Reset the edge capture register. */
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(PIO_INTERRUPT_BASE, 0x0);
	/* Register the ISR. */
	alt_ic_isr_register(PIO_INTERRUPT_IRQ_INTERRUPT_CONTROLLER_ID, PIO_INTERRUPT_IRQ, handle_button_interrupts, edge_capture_ptr, 0x0);

}

void process_no_interrupt()
{
	printf("interrupt disable\n");
	alt_ic_irq_disable(PIO_INTERRUPT_IRQ_INTERRUPT_CONTROLLER_ID, PIO_INTERRUPT_IRQ);

	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, 0x01);
	usleep(20000000);
	IOWR_ALTERA_AVALON_PIO_DATA(PIO_0_BASE, 0x00);
	alt_ic_irq_enable(PIO_INTERRUPT_IRQ_INTERRUPT_CONTROLLER_ID, PIO_INTERRUPT_IRQ);
	printf("interrupt enable\n");
}

int GetMSB(int intValue)
{
    return (intValue & 0xFFFF0000) >> 16;
}

int GetLSB(int intValue)
{
    return (intValue & 0x0000FFFF);
}

void read_on_chip_ram(int ram_addr , Point* src_point, Point* dst_point)
{
	//uint32_t xy;
	uint32_t read_val;
	for (int idx = 0; idx < MATCHES_NUMBER*2 ; idx++  )
	{
		read_val = IORD(ram_addr, 0);

		src_point[idx].y = GetMSB(read_val);
		src_point[idx].x = GetLSB(read_val);

	}

}

void addArrayPointsX(Point a[],Point b[],Point res[],int size)
{
    int i;
    for(i=0; i< size;i++)
    	res[i].x = a[i].x + b[i].x;
}


int main()
{
	init_button_pio();
	printf("Hello from Nios II!\n");
	if (alt_timestamp_start() < 0) {
		printf("Timer init failed \n");
	}
			//calculate overhead
	int time1 = alt_timestamp();
	int time2 = alt_timestamp();
	int num_ticks;
	int timer_overhead = time2 - time1;
	printf("Number of ticks per second = %u \n", (unsigned int)alt_timestamp_freq());

	int array_size = MATCHES_NUMBER;
	Point src_points[MATCHES_NUMBER];
	Point dst_points[MATCHES_NUMBER];
	Point u[MATCHES_NUMBER];

	int int_counter = 0;

	while(1)
			{
				time1 = alt_timestamp();

				read_on_chip_ram(ONCHIP_MEMORY2_1_BASE, src_points, dst_points);

				addArrayPointsX(src_points, dst_points, u, array_size);
				printf("mem x = %x, y = %x\n", src_points[0].y, src_points[0].x);

				time2 = alt_timestamp(); // end timer

				num_ticks = time2 - time1 - timer_overhead; // calculate number of ticks

				printf("Time in ticks = %u \n", (unsigned int)num_ticks);
				if (edge_capture == 1){
					printf("edge_capture count = %d\n", int_counter);
					int_counter++;
					process_no_interrupt();
					edge_capture = 0;
				}


		       //IOWR_ALTERA_AVALON_PIO_DATA(0x81000, count & 0x01);
		   	   //usleep(20000);
		   	   //printf("test.x = %d, test.y = %ld\n", test[0].x , test[0].y );

		   }
  return 0;
}
