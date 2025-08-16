#ifndef _UTILS_H_
#define _UTILS_H_

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifndef _countof
	#define _countof(_Array) (sizeof(_Array) / sizeof(_Array[0]))
#endif

#if !defined(min) || !defined(max)
	#define max(a,b) (((a) > (b)) ? (a) : (b))
	#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif

// returns a string based on ndpi_protocol_ids.h or 
const char * ndpi_protocol_id_to_str(uint16_t proto_id);

// print flow info to be used for debugging and tracing why a specific app is not detected
void print_flow_info_debug(struct ndpi_flow_info *flow);
#endif