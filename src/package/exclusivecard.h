#ifndef EXCLUSIVECARD_H
#define EXCLUSIVECARD_H

#include "standard.h"

class ExclusiveCardPackage: public Package{
    Q_OBJECT

public:
    ExclusiveCardPackage();
};

class Chaoxiu: public Weapon{
    Q_OBJECT

public:
    Q_INVOKABLE Chaoxiu(Card::Suit suit, int number);
    
    virtual QString getSubtype() const;
};

#endif // EXCLUSIVECARD_H
