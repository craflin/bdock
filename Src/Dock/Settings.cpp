
#include "stdafx.h"

Settings::Settings(Storage& storage) : 
barHeight(storage.getInt("barHeight", 57)),
leftMargin(storage.getInt("leftMargin",25)),
topMargin(storage.getInt("topMargin",10)),
rightMargin(storage.getInt("rightMargin",25)),
bottomMargin(storage.getInt("bottomMargin",2)),
itemWidth(storage.getInt("itemWidth",42)),
iconWidth(storage.getInt("iconWidth",32)),
iconHeight(storage.getInt("iconHeight",32)),
alignment(Alignment(storage.getInt("alignment", center))),
edge(Alignment(storage.getInt("edge", bottom)))
{}
