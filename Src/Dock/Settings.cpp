
#include "stdafx.h"

Settings::Settings(Storage& storage) : 
barHeight(storage.getInt(_T("barHeight"), 57)),
leftMargin(storage.getInt(_T("leftMargin"), 25)),
topMargin(storage.getInt(_T("topMargin"), 10)),
rightMargin(storage.getInt(_T("rightMargin"), 25)),
bottomMargin(storage.getInt(_T("bottomMargin"), 2)),
itemWidth(storage.getInt(_T("itemWidth"), 42)),
iconWidth(storage.getInt(_T("iconWidth"), 32)),
iconHeight(storage.getInt(_T("iconHeight"), 32)),
alignment(Alignment(storage.getInt(_T("alignment"), center))),
edge(Alignment(storage.getInt(_T("edge"), bottom)))
{}
