#ifndef POSITION_ENTRY_H_
#define POSITION_ENTRY_H_

struct PositionEntry {
    double tStart;    
    double tEnd;       
    double lat;    
    double lon;  

    bool contains(int t) const {
        return t >= tStart && t < tEnd;
    }
};

#endif /* POSITION_ENTRY_H_ */