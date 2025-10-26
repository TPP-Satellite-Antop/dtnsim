#ifndef SRC_NODE_MSGTYPES_H_
#define SRC_NODE_MSGTYPES_H_

// Messages types and their priority are
// equal, unique and defined in this file.
// Higher scheduling priority (smaller
// numeric value) are executed first.

#define DISC_CONTACT_END_TIMER 0
#define DISC_CONTACT_START_TIMER 1

#define CONTACT_END_TIMER 2
#define CONTACT_START_TIMER 3

#define DISC_CONTACT_NEIGHBOR_END_TIMER 4
#define DISC_CONTACT_NEIGHBOR_START_TIMER 5

#define CONTACT_FAILED 6

#define TRAFFIC_TIMER 7

#define BUNDLE_CUSTODY_REPORT 8 // white
#define BUNDLE 9                // yellow

#define CUSTODY_TIMEOUT 10

#define FORWARDING_MSG_END 11
#define FORWARDING_MSG_START 12

#define FAULT_START_TIMER 20
#define FAULT_END_TIMER 21

#define SCHEDULING_RETRY 30

#endif /* SRC_NODE_MSGTYPES_H_ */
