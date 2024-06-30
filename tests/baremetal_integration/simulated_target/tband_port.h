#ifndef TBAND_PORT_H_
#define TBAND_PORT_H_

#include "mocks.h"

#define tband_portENTER_CRITICAL_FROM_ANY()
#define tband_portEXIT_CRITICAL_FROM_ANY()

#define tband_portTIMESTAMP_RESOLUTION_NS 1

#define tband_portTIMESTAMP() mock_port_ts()

#define tband_portNUMBER_OF_CORES (1)

#define tband_portGET_CORE_ID() (0)

#endif /* TBAND_PORT_H_ */
