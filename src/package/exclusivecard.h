#ifndef EXCLUSIVECARD_H
#define EXCLUSIVECARD_H

#include "meleecard.h"

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

class ViolentMask: public Armor{
    Q_OBJECT

public:
    Q_INVOKABLE ViolentMask(Card::Suit suit, int number);
    
    virtual QString getSubtype() const;
};

class ShamanTotem: public Relic{
    Q_OBJECT

public:
    Q_INVOKABLE ShamanTotem(Card::Suit suit, int number);
    
    virtual QString getSubtype() const;
};

#endif // EXCLUSIVECARD_H
