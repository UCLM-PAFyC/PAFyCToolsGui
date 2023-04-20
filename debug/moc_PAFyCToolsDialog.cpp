/****************************************************************************
** Meta object code from reading C++ file 'PAFyCToolsDialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../PAFyCToolsDialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'PAFyCToolsDialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_PAFyCToolsDialog_t {
    QByteArrayData data[10];
    char stringdata0[252];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PAFyCToolsDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PAFyCToolsDialog_t qt_meta_stringdata_PAFyCToolsDialog = {
    {
QT_MOC_LITERAL(0, 0, 16), // "PAFyCToolsDialog"
QT_MOC_LITERAL(1, 17, 29), // "on_qgisPathPushButton_clicked"
QT_MOC_LITERAL(2, 47, 0), // ""
QT_MOC_LITERAL(3, 48, 31), // "on_outputPathPushButton_clicked"
QT_MOC_LITERAL(4, 80, 38), // "on_commandComboBox_currentInd..."
QT_MOC_LITERAL(5, 119, 5), // "index"
QT_MOC_LITERAL(6, 125, 25), // "on_helpPushButton_clicked"
QT_MOC_LITERAL(7, 151, 31), // "on_parametersPushButton_clicked"
QT_MOC_LITERAL(8, 183, 39), // "on_ProgressExternalProcessDia..."
QT_MOC_LITERAL(9, 223, 28) // "on_processPushButton_clicked"

    },
    "PAFyCToolsDialog\0on_qgisPathPushButton_clicked\0"
    "\0on_outputPathPushButton_clicked\0"
    "on_commandComboBox_currentIndexChanged\0"
    "index\0on_helpPushButton_clicked\0"
    "on_parametersPushButton_clicked\0"
    "on_ProgressExternalProcessDialog_closed\0"
    "on_processPushButton_clicked"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PAFyCToolsDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    1,   51,    2, 0x08 /* Private */,
       6,    0,   54,    2, 0x08 /* Private */,
       7,    0,   55,    2, 0x08 /* Private */,
       8,    0,   56,    2, 0x08 /* Private */,
       9,    0,   57,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    5,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PAFyCToolsDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PAFyCToolsDialog *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_qgisPathPushButton_clicked(); break;
        case 1: _t->on_outputPathPushButton_clicked(); break;
        case 2: _t->on_commandComboBox_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->on_helpPushButton_clicked(); break;
        case 4: _t->on_parametersPushButton_clicked(); break;
        case 5: _t->on_ProgressExternalProcessDialog_closed(); break;
        case 6: _t->on_processPushButton_clicked(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject PAFyCToolsDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_PAFyCToolsDialog.data,
    qt_meta_data_PAFyCToolsDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PAFyCToolsDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PAFyCToolsDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PAFyCToolsDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int PAFyCToolsDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
