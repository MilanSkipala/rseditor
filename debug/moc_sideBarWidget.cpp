/****************************************************************************
** Meta object code from reading C++ file 'sideBarWidget.h'
**
** Created: Mon 16. Sep 18:52:12 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../sideBarWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'sideBarWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SideBarWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,
      31,   14,   14,   14, 0x0a,
      47,   14,   14,   14, 0x0a,
      64,   14,   14,   14, 0x0a,
      81,   14,   14,   14, 0x0a,
      99,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_SideBarWidget[] = {
    "SideBarWidget\0\0showAddDialog()\0"
    "showDelDialog()\0closeAddDialog()\0"
    "closeDelDialog()\0importSelection()\0"
    "deleteSelection()\0"
};

void SideBarWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SideBarWidget *_t = static_cast<SideBarWidget *>(_o);
        switch (_id) {
        case 0: _t->showAddDialog(); break;
        case 1: _t->showDelDialog(); break;
        case 2: _t->closeAddDialog(); break;
        case 3: _t->closeDelDialog(); break;
        case 4: _t->importSelection(); break;
        case 5: _t->deleteSelection(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData SideBarWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SideBarWidget::staticMetaObject = {
    { &QFrame::staticMetaObject, qt_meta_stringdata_SideBarWidget,
      qt_meta_data_SideBarWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SideBarWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SideBarWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SideBarWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SideBarWidget))
        return static_cast<void*>(const_cast< SideBarWidget*>(this));
    return QFrame::qt_metacast(_clname);
}

int SideBarWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
