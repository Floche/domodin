#ifndef OUTPUTS_H
#define	OUTPUTS_H

#ifndef bool
#define bool int1
#endif

enum t_Order{
    ARRET = 0, HORS_GEL = 1, ECO = 2, CONFORT = 3 
};

typedef struct {
    t_Order CurrentOrder;
    unsigned long Positive;
    unsigned long Negative;
}t_Radiator;

void Init_outputs();
void Radiator(t_Order Order, int8 id);
void Detection_delestage();
void Update_outputs();
void Update_Relais(bool bRelai_1, bool bRelai_2);

#endif	/* OUTPUTS_H */

