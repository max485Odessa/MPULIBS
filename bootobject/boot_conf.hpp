#ifndef SRC_TBOOTCORE_CONFIG_HPP_
#define SRC_TBOOTCORE_CONFIG_HPP_


#define C_PROJECT_PASSWORD "3GChanSelTargTuk"
#define C_PROJECT_DEVNAME "ant-tracker"

#define C_USER_PROGRAMM_ADRESS (uint32_t)0x08008000		// адрес старта програмы в STM32
#define C_FIRMWARE_ARRTAG_SIZE (65536*4)				// максимально возможный размер пользовательской программы (для доп контроля целостности прошивки и расчета масива резервных копий прошивок хранящихся во внешней EEPROM)

// EEPROM адреса
#define C_EEPROM_FMW_REQ_TAG_ADRESS 0x2000	// тег запроса
#define C_EEPROM_FMW_USER_PARAM_TAG_ADR 0x4000	// параметры текущей прошивки залитой в STM32
#define C_EEPROM_FMW_BOOT_MSG_ADR 0x6000	// сообщение которое оставляет бут о результате обновления
#define C_EEPROM_FMW_ARRAY_ADR 0x8000	// стартовый адрес массива копий прошивок
#define C_EEPROM_FMW_ARRAY_CNT 2	// количество прошивок

#define C_EEPROM_FREE_SPACE (C_EEPROM_FMW_ARRAY_ADR + C_FIRMWARE_ARRTAG_SIZE * C_EEPROM_FMW_ARRAY_CNT)

#define C_CURENT_BOOT_SIZE 32768

#endif
