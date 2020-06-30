#include "stm32f1xx.h"
#include "modem.h"
#include "errors.h"
#include "call.h"
#include "main.h"
#include "atCommands.h"
#include "systick.h"

void show_state();

const uint32_t LOCK_TIMEOUT = 20 * 60 * 1000u;
const uint32_t UNLOCK_TIMEOUT = 2 * 60 * 1000;

errorcode error = 0;

int main(void)
{
	errorcode result = modem_init();
	if(result!= ERR_NOERROR)
		error = result;

	show_state();

	while(1)
	{
		process_call();
		processsms();
		show_state();
	}
}

//�������� ����������
//delay - ����� ������ � �������
void alg(uint16_t delay)
{

	//���� 1
 	GPIOB->BSRR = 0x03000080; //green on

 	volatile uint32_t timestamp = getSystime();
	while(!IsLocked() && !checkDelay(timestamp, LOCK_TIMEOUT));

	if(!IsLocked())
	{
		//����-���
		GPIOB->BSRR = 0x03880000; //all off
		return;
	}

	GPIOB->BSRR = 0x00000100; //orange on

	//���� 2
	timestamp = getSystime();
	volatile uint32_t timestamp2 = getSystime(); //������� ����� ��� �������� ���������� ���������

	while(!checkDelay(timestamp, delay*60u*1000u))
	{
		if(GPIOB->IDR & 0x0040)
		{
			//������ ���������� ����
			if(!(GPIOB->ODR & 0x00000008))
			{
				//�����
				GPIOB->BSRR = 0x00000208; //red, out on
			}
		}
		else
		{
			//������� ���������� ���
			if(GPIOB->ODR & 0x00000008)
			{
				//����
				GPIOB->BSRR = 0x02080000; //red, out off
				timestamp2 = getSystime();
			}

			//���� ������� ���������� ��� ��� unlockexittime ��
			if(checkDelay(timestamp2, UNLOCK_TIMEOUT))
				break;
		}
	}

	GPIOB->BSRR = 0x03880000; //all off

	//������
	sendcommand(callCommand, 20000);
}

//���������� ������ �� �����������
void show_state()
{
	if(error == ERR_NOERROR)
		GPIOB->BSRR = 0x03800000;
	else
		GPIOB->BSRR = 0x01800200; //red on
}
