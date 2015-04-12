#include <Python.h>
// 设置 UNICODE 库，这样的话才可以正确复制宽字符集
#define UNICODE


#include "debugoutput.h"

#include "ctrlengine.h"
#include "callargument.h"

#include "utils.h"


#include "pyinit.h"


// 扩展入口函数Init_extname
static PyInit *pyinit = NULL;

extern "C" void Init_forpy()
{
    PyInit *init = new PyInit();
    init->initialize();
    pyinit = init;
}

// #define PyMODINIT_FUNC extern "C" PyObject*
extern "C" PyObject* PyInit_qt5()
{
    Init_forpy();
    return pyinit->m_cModuleQt;
}

/////////////
// 参数格式，QStringList* argv, PyObject* self
static PyObject* px_Qt_class_missing(int argc, void* argv, void* self)
{ return pyinit->Qt_class_missing(argc, argv, self); }

/*
static VALUE nx_Qt_constant_missing(int argc, VALUE* argv, VALUE self)
{ return rbinit->Qt_constant_missing(argc, argv, self); }

static VALUE nx_Qt_method_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_method_missing(argc, argv, self); }

// for qApp 等全局变量
static VALUE nx_Qt_global_variable_get(ID id, VALUE *data, struct global_entry *entry)
{ return rbinit->Qt_global_variable_get(id, data, entry); }

static void nx_Qt_global_variable_set(VALUE value, ID id, VALUE *data, struct global_entry *entry)
{ rbinit->Qt_global_variable_set(value, id, data, entry); }

static VALUE nx_Qt_class_new(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_new(argc, argv, self); }

static VALUE nx_Qt_class_init(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_init(argc, argv, self); }

static VALUE nx_Qt_class_dtor(VALUE id)
{ return rbinit->Qt_class_dtor(id); }

static VALUE nx_Qt_class_to_s(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_to_s(argc, argv, self); }

static VALUE nx_Qt_class_const_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_const_missing(argc, argv, self); }

static VALUE nx_Qt_class_method_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_method_missing(argc, argv, self); }

static VALUE nx_Qt_class_singleton_method_missing(int argc, VALUE *argv, VALUE self)
{ return rbinit->Qt_class_singleton_method_missing(argc, argv, self); }
*/

// 定义 python 的 model
static struct PyModuleDef abModule = { PyModuleDef_HEAD_INIT, "ab", NULL, -1, NULL};
static struct PyModuleDef qtModule = { PyModuleDef_HEAD_INIT, "qt", NULL, -1, NULL};
static struct PyModuleDef qt5Module = { PyModuleDef_HEAD_INIT, "qt5", NULL, -1, NULL};

#define MODDEF(pmname) \
    static struct PyModuleDef pmname##Module = { PyModuleDef_HEAD_INIT, ""#pmname, NULL, -1, NULL };

PyObject py_object_initializer =  {
    _PyObject_EXTRA_INIT
    1,
    NULL    // type must be init'ed by user
};

#include <frameobject.h>
_Py_IDENTIFIER(excepthook);
_Py_IDENTIFIER(extract_tb);
_Py_IDENTIFIER(type);

extern "C" PyObject* qgc_excepthook(PyObject* self, PyObject* args)
{
    qDebug()<<"catched sys.excepthook call:"<<self<<args;
    assert(self == pyinit->m_cModuleQt);
    
    PyObject *exc, *value, *tb;
    if (!PyArg_UnpackTuple(args, "excepthook", 3, 3, &exc, &value, &tb))
        return NULL;
    
    qDebug()<<exc<<value<<tb;
    /*
    PyObject_Print(exc, stdout, 1); puts("\n");
    PyObject_Print(value, stdout, 1); puts("\n");
    PyObject_Print(tb, stdout, 1); puts("\n");
    PyObject_Print(PyObject_Type(exc), stdout, 1); puts("aaaaaaaaa\n");
    PyObject_Print(PyObject_Type(value), stdout, 1); puts("aaaaaaaaa\n");
    
    PyTracebackObject *tbo = (PyTracebackObject*)tb;
    PyFrameObject *fro = tbo->tb_frame;
    PyCodeObject *coo = fro->f_code;
    PyObject_Print(coo->co_code, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_consts, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_name, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_names, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_varnames, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_cellvars, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(coo->co_filename, stdout, 1); puts("aaaaaaaaaa\n");

    PyBaseExceptionObject* bee = (PyBaseExceptionObject*)exc;
    PyObject_Print(bee->dict, stdout, 1); puts("aaaaaaaaaa\n");
    // PyObject_Print(bee->args, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(bee->context, stdout, 1); puts("aaaaaaaaaa\n");
    PyObject_Print(bee->cause, stdout, 1); puts("aaaaaaaaaa\n");
    */
    
    auto getAttrByExcValue = [](PyObject* excval) -> auto {
        QRegExp exp("'module' object has no attribute '(.*)'");
        QString valstr = PyUnicode_AsUTF8(PyObject_Str(excval));
        auto mats = exp.exactMatch(valstr);
        // qDebug()<<mats<<valstr<<exp.cap(1);

        return exp.cap(1);
    };
    auto attrName = getAttrByExcValue(value);
    // qDebug()<<attrName;

    auto getAttrStart = [](QString &line, QString &attrName) -> auto {
        QStringList segs = line.split(' ');
        QString start;
        for (QString &seg: segs) {
            if (seg.indexOf(attrName) != -1) {
                start = seg;
                break;
            }
        }
        return start;
    };

    // PyImport_ImportModuleNoBlock("traceback");
    PyImport_ImportModule("traceback");
    PyObject* tbm = PyImport_ImportModuleNoBlock("traceback");
    _object* rl = _PyObject_CallMethodId(tbm, &PyId_extract_tb, "O", tb);
    // PyObject_Print(rl, stdout, 1); puts("aaaaaaaaaa\n");
    // qDebug()<<PyList_Size(rl);
    PyObject* rt = PyList_GetItem(rl, 0); // a tuple object
    // PyObject_Print(rt, stdout, 1); puts("aaaaaaaaaa\n");
    // qDebug()<<PyTuple_Size(rt);
    PyObject* rs = PyTuple_GetItem(rt, 3);
    // PyObject_Print(rs, stdout, 1); puts("aaaaaaaaaa\n");
    // qDebug()<<PyUnicode_AsUTF8(rs);
    QString eline = PyUnicode_AsUTF8(rs);

    // TODO 现在只是定位到一行，还需要更进一步准确定位。
    // 例如，a = eg.QString123()
    // AttributeError: 'module' object has no attribute 'QString123'
    // 对于AttributeError的情况，匹配出来attribute名字，在这一行中回溯到非空格位置，
    // 然后再用以下检测方法检测。
    QString fullAttr = getAttrStart(eline, attrName);
    if (fullAttr.startsWith("qt5.Q")) {
        qDebug()<<"got a class call"<<fullAttr;
        QString klass = eline.split(".").at(1);
        strcpy((char*)0x123, (char*)0x456);
        return PyLong_FromLong(5);
        return Py_None;// 返回Py_None的时候，程序就结束了。怎么能让程序继续执行呢？
    } else if (fullAttr.startsWith("qt5.q")) {
        qDebug()<<"maybe it's a function"<<fullAttr;
        QString fname = eline.split(".").at(1);
        
        return Py_None;
    } else if (fullAttr.startsWith("qt5.")) {
        qDebug()<<"maybe it's a const"<<fullAttr;        
        QString cname = eline.split(".").at(1);
        
        return Py_None;
    } else {
        qDebug()<<"not careeeeeeeee"<<fullAttr;
        // not care, goon
    }
    
    PyErr_Display(exc, value, tb);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject* (*oldgetattrfn)(PyObject*, PyObject*) = NULL;
extern "C" PyObject* mygetattr(PyObject* o, char *a)
{
    qDebug()<<o<<a<<oldgetattrfn;

    return PyObject_GenericGetAttr(o, PyUnicode_InternFromString(a));
}

extern "C" PyObject* mygetattro(PyObject* o, PyObject* a)
{
    // qDebug()<<o<<a;
    //NOTE: 如果有别的程序改了tp_getattro则这个assert不成立了。
    assert(oldgetattrfn == PyObject_GenericGetAttr);
    PyTypeObject *tp = Py_TYPE(o);
    if (strcmp(tp->tp_name, "module") == 0 && strcmp(PyModule_GetName(o), "qt5") == 0) {
        qDebug()<<_PyUnicode_AsString(a)<<PyErr_Occurred();
    }
    PyObject* reto = PyObject_GenericGetAttr(o, a);
    // 如果是reto是NULL的话，已经设置了错误信息了，所以会输出错误。这个错误是否能够清除掉呢？

    if (strcmp(tp->tp_name, "module") == 0 && strcmp(PyModule_GetName(o), "qt5") == 0) {
        qDebug()<<reto<<_PyUnicode_AsString(a)<<PyErr_Occurred();
    }
    if (reto) return reto;

    // hacked getattr...
    auto Hack_GenericGetAttr = [a](PyObject* v, PyObject* name) -> PyObject* {
        const char *vstr;
        PyTypeObject *tp = Py_TYPE(v);        
        if (!v) return NULL;
        vstr = tp->tp_name;

        // 试验代码
        if (strcmp(vstr, "module") == 0) {
            if (strcmp(PyModule_GetName(v), "qt5") == 0) {
                printf("inhackkkkkkkk:%s::%s\n", PyModule_GetName(v), _PyUnicode_AsString(name));
                if (strcmp(_PyUnicode_AsString(name), "abcdefg") == 0) {
                    printf("inhackkkkkkkk:%s\n", PyModule_GetName(v));
                }
            }
        }

        // 不满足条件则返回
        if (strcmp(vstr, "module") == 0 && strcmp(PyModule_GetName(v), "qt5") == 0) {
        } else {
            qDebug()<<"omited..."<<vstr<<PyModule_GetName(v);
            return NULL;
        }
        
        
        QString tpName = QString(tp->tp_name);
        QString modName = QString(PyModule_GetName(v));
        QString attrName = _PyUnicode_AsString(name);
        qDebug()<<tpName<<modName<<attrName;

        if (attrName.startsWith("Q") && attrName.at(1).isUpper()) {
            qDebug()<<"got a class call"<<attrName;
            QString klass = attrName;
            QStringList argv = {attrName};
            PyObject* dict = *_PyObject_GetDictPtr(v);
            qDebug()<<_PyObject_GetDictPtr(v)<<PyDict_Size(dict)
                    <<PyDict_GetItem(dict, a);
            PyObject* ret = px_Qt_class_missing(1, &argv, v);
            qDebug()<<_PyObject_GetDictPtr(v)<<PyDict_Size(dict)
                    <<PyDict_GetItem(dict, a);
            PyObject* res = PyDict_GetItem(dict, a);
            Py_INCREF(res);
            return res;
            return ret;
        } else if (attrName.startsWith("q") && attrName.at(1).isUpper()) {
            qDebug()<<"maybe it's a function"<<attrName;
            QString fname = attrName;
        
        } else if (attrName.at(1).isUpper()) {
            qDebug()<<"maybe it's a const"<<attrName;
            QString cname = attrName;
        
        } else {
            qDebug()<<"not careeeeeeeee"<<attrName;
            // not care, goon
        }
        
        return NULL;
    };

    qDebug()<<"missing sth...";
    reto = Hack_GenericGetAttr(o, a);
    qDebug()<<reto;
    PyObject_GenericSetAttr(o, a, reto);
    reto = PyObject_GenericGetAttr(o, a);
    qDebug()<<reto<<PyErr_Occurred();
    PyErr_Clear(); // 非常重要，清除函数开始时设置的错误标识信息。
    qDebug()<<reto<<PyErr_Occurred();
    return reto;
    return NULL;
}

//_Py_static_string(PyId_excepthook, excepthook);
void PyInit::initialize()
{
    qInstallMessageHandler(myMessageOutput);
    gce = new CtrlEngine();
    Py_Initialize();

    // 注册qt5模块，即qt5.py
    PyObject* cModuleQt = NULL;
    cModuleQt = PyModule_Create(&qt5Module);
    m_cModuleQt = cModuleQt;
    PyTypeObject* mto = (PyTypeObject*)PyObject_Type(cModuleQt);
    oldgetattrfn = mto->tp_getattro; // 保存下默认的函数
    qDebug()<<mto->tp_getattr<<oldgetattrfn;
    mto->tp_getattr = mygetattr;  // 真的管用啊，注入式的。
    mto->tp_getattro = mygetattro;  // 真的管用啊，注入式的。
    qDebug()<<PyUnicode_AsUTF8(PyObject_Str((PyObject*)mto));

    int ok = false;
    ok = PyModule_Check(cModuleQt);

    qDebug()<<"mc.....:"<<ok<<PyModule_GetName(cModuleQt);
        // 这一行导致一个报错：SystemError: initialization of qt5 raised unreported exception
        // 但是如果有注册的类之后，这个却不会有什么问题。
    // <<"'"<<PyModule_GetFilenameObject(cModuleQt)<<"'";
    qDebug()<<_Py_PackageContext;

    // for little testing.
    this->registClass("QString123");
    this->registClass("QString456");
    this->registClass("QString789");
    PyTypeObject* q456 = this->m_classes.value("QString789");
    PyObject_Print((PyObject*)q456, stdout, 0); puts("\n");

    // 注册类方法不存在事件处理
    // 这里需要一个模块级的sys.excepthook
    auto hook = _PySys_GetObjectId(&PyId_excepthook);
    // PyObject_Print((PyObject*)hook, stdout, 1);
    // puts("\n");

    // _PySys_SetObjectId(&PyId_excepthook, NULL); // 置空
    static PyMethodDef exh = {"pqc_excepthook", qgc_excepthook, METH_VARARGS, "qgc_excepthook"};
    PyObject* exh_fn = PyCFunction_NewEx(&exh, cModuleQt, NULL);
    // PyObject_Print((PyObject*)exh_fn, stdout, 1);
    // puts("\n");
    qDebug()<<METH_VARARGS<<PyCFunction_GET_FLAGS(exh_fn)
            <<(PyCFunction_GET_FLAGS(exh_fn) & ~(METH_CLASS | METH_STATIC | METH_COEXIST));
    // _PySys_SetObjectId(&PyId_excepthook, exh_fn);// depcreated way

    hook = _PySys_GetObjectId(&PyId_excepthook);
    // PyObject_Print((PyObject*)hook, stdout, 1);
    // puts("\n");
    
    qDebug()<<"Hhhhhhhhh";
    /*
    // 对所有的Qt5::someconst常量的调用注册
    rb_define_module_function(cModuleQt, "const_missing", FUNVAL nx_Qt_constant_missing, -1);
    // 对所有的Qt5::somefunc()函数的调用注册
    rb_define_module_function(cModuleQt, "method_missing", FUNVAL nx_Qt_method_missing, -1);
  
    // for qApp 类Qt全局变量
    rb_define_virtual_variable("$qApp", (VALUE (*)(ANYARGS)) nx_Qt_global_variable_get,
                               (void (*)(ANYARGS)) nx_Qt_global_variable_set);
    */
}

//Py_Finalize();

extern "C" void standard_dealloc( PyObject *p )
{
    qDebug()<<p;
    // PyMem_DEL( p );
}

extern "C" PyObject* standard_new(PyTypeObject* subtype, PyObject *args, PyObject* kwds)
{
    qDebug()<<subtype<<args<<kwds;
    qDebug()<<PyUnicode_AsUTF8(PyObject_Str((PyObject*)subtype))
            <<PyUnicode_AsUTF8(PyObject_Str((PyObject*)args))<<"eee";
    // PyMem_DEL( p );

    PyObject* self = subtype->tp_alloc(subtype, 0);
    qDebug()<<self;
    Py_INCREF(self);
    
    return self;
    return PyLong_FromLong(5);
    return NULL;
}

extern "C" int standard_init(PyObject* self, PyObject *args, PyObject* kwds)
{
    qDebug()<<self<<args<<kwds;

    return -1;
    return 0;
    return 156789;
}

extern "C" void standard_free(void *p)
{
    qDebug()<<p;
    // PyMem_DEL( p );
}

struct QString123{
};

// file:///usr/share/doc/python/html/extending/newtypes.html?highlight=type
typedef struct {
    PyObject_HEAD
    /* Type-specific fields go here. */
    void *inst;
} qt5_QKlassObject;

static PyTypeObject qt5_QKlassTypeTemplate = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "qt5.QKlass",                      /* tp_name */
    sizeof(qt5_QKlassObject), /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "QKlass objects template",           /* tp_doc */
};


bool PyInit::registClass(QString klass)
{
    // see    ExtensionType.hxx:212  static PythonType &behaviors()
    //*
    auto NewTypeObject_by_template = [](QString klass) -> PyTypeObject* {
        static_assert(sizeof(qt5_QKlassTypeTemplate) == sizeof(PyTypeObject));
        PyTypeObject* tbl = (PyTypeObject*)calloc(1, sizeof(PyTypeObject));
        memcpy(tbl, &qt5_QKlassTypeTemplate, sizeof(PyTypeObject));
        
        tbl->tp_name = strdup(QString("qt5.%1").arg(klass).toLatin1().data());
        tbl->tp_doc = strdup(QString("qt5.%1 objects").arg(klass).toLatin1().data());

        tbl->tp_new = PyType_GenericNew;

        if (PyType_Ready(tbl) < 0) {
            qDebug()<<"type not ready:"<<tbl->tp_name;
            assert(0);
        }
        Py_INCREF(tbl);
        qDebug()<<tbl<<tbl->tp_basicsize<<tbl->tp_doc;
        return tbl;
    };
    auto NewTypeObject_by_call_buildin_type
        = [](QString klass) -> PyTypeObject* {
        //  PyType_Type();
        return NULL;
    };
    auto NewTypeObject = [](QString klass) -> auto /* PyTypeObject* */ {
        // auto tbl = new PyTypeObject();
        PyTypeObject* tbl = (PyTypeObject*)calloc(1, sizeof(PyTypeObject));
        memset(tbl, 0, sizeof(PyTypeObject));
        *reinterpret_cast<PyObject*>(tbl) = py_object_initializer;
        reinterpret_cast<PyObject*>(tbl)->ob_type = &PyType_Type;
        // tbl->tp_name = "qt5.QString456"; // 应该用qt5.QString还是QString呢？用<module>.<name>
        tbl->tp_name = strdup(QString("qt5.%1").arg(klass).toLatin1().data());
        tbl->tp_basicsize = 80;
        tbl->tp_itemsize = 0;
        tbl->tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
        // tbl->tp_dealloc = standard_dealloc;
        tbl->tp_new = standard_new;
        // tbl->tp_free = standard_free;
        // tbl->tp_init = standard_init;
        // tbl->tp_dealloc = extension_object_deallocator;
        //*/

        // table->tp_name = const_cast<char *>( default_name );
        // table->tp_basicsize = basic_size;
        // table->tp_itemsize = itemsize;

        // p->set_tp_new( extension_object_new );
        // p->set_tp_init( extension_object_init );
        // p->set_tp_dealloc( extension_object_deallocator );
        
        qDebug()<<"ty ready???"<<PyType_Ready(tbl); // 这个函数的调用很重要啊，相当于类型定义结束。必须要执行的。
        Py_INCREF(tbl);

        return tbl;
    };
    // auto tbl = NewTypeObject(klass);
    auto tbl2 = NewTypeObject_by_template(klass);
    
    // PyModule_AddObject(cModuleQt, "QString456", (PyObject*)tbl);
    // PyModule_AddObject(m_cModuleQt, strdup(QString("%1").arg(klass).toLatin1().data()), (PyObject*)tbl);
    PyModule_AddObject(m_cModuleQt, strdup(QString("%1").arg(klass).toLatin1().data()), (PyObject*)tbl2);
    int ok = PyModule_Check(m_cModuleQt);
    qDebug()<<ok<<m_cModuleQt<<PyModule_CheckExact(m_cModuleQt);
    // PyObject_Print((PyObject*)tbl, stdout, 1);
    // puts("\n");
    // PyObject_Print((PyObject*)m_cModuleQt, stdout, 1);
    // puts("\n");

    this->m_classes[klass] = tbl2;

    return true;
}

// getattr应该返回这样的结果：<class 'type'>
PyObject* PyInit::Qt_class_missing(int argc, void* argv, void* self)
{
    qDebug()<<argc<<argv<<self;
    assert(argc == 1);
    QStringList* args = (QStringList*)argv;
    QString klass = args->at(0);
    qDebug()<<klass;

    this->registClass(klass);
    PyTypeObject* cKlassQt = this->m_classes.value(klass);
    qDebug()<<this->m_classes.size()<<cKlassQt
            <<PyUnicode_AsUTF8(PyObject_Str((PyObject*)cKlassQt));

    descrgetfunc f = ((PyObject*)cKlassQt)->ob_type->tp_descr_get;
    qDebug()<<f<<_PyUnicode_AsString(PyObject_Str((PyObject*)cKlassQt))<<"ee___eeee";
    // assert(f != NULL);
    // Py_XINCREF(cKlassQt);
    qDebug()<<this->m_classes.value("QString123");
    qDebug()<<this->m_classes.value("QString456");
    qDebug()<<this->m_classes.value("QString789");

    return (PyObject*)cKlassQt;
    return NULL;
}

////// tryingggggggggg codessssssssssss
extern "C" void mymodule_dealloc( void *p )
{
    qDebug()<<"dealloccccccccing:"<<p;
}

// #define PyMODINIT_FUNC extern "C" PyObject*
extern "C" PyObject* PyInit_ab()
{
    Init_forpy();

    return pyinit->m_cModuleQt;
    MODDEF(abcdefg);
    // MODDEF(qt);
    // MODDEF(qt5);
    /*
    PyObject *pyoab = PyModule_Create(&abModule);
    PyObject *pyoqt = PyModule_Create(&qtModule);
    PyObject *pyoqt5 = PyModule_Create(&qt5Module);
    
    pyinit->m_cModuleQt = pyoqt5;
    return pyinit->m_cModuleQt;
    */
}

extern "C" void dbg_type(void *to)
{
    qDebug()<<to;
}

////////////////

static PyTypeObject* make_type(char *type, PyTypeObject* base, char**fields, int num_fields)
{
    PyObject *fnames, *result;
    int i;
    fnames = PyTuple_New(num_fields);
    if (!fnames) return NULL;
    for (i = 0; i < num_fields; i++) {
        PyObject *field = PyUnicode_FromString(fields[i]);
        if (!field) {
            Py_DECREF(fnames);
            return NULL;
        }
        PyTuple_SET_ITEM(fnames, i, field);
    }
    result = PyObject_CallFunction((PyObject*)&PyType_Type, "s(O){sOss}",
                    type, base, "_fields", fnames, "__module__", "_ast");
    Py_DECREF(fnames);
    return (PyTypeObject*)result;
}



extern "C" void extension_object_deallocator( PyObject *_self )
{
    qDebug()<<"dealloccccccccing:"<<_self;
    /*
    PythonClassInstance *self = reinterpret_cast< PythonClassInstance * >( _self );
#ifdef PYCXX_DEBUG
    std::cout << "extension_object_deallocator( self=0x" << std::hex << reinterpret_cast< unsigned int >( self ) << std::dec << " )" << std::endl;
    std::cout << "    self->m_pycxx_object=0x" << std::hex << reinterpret_cast< unsigned int >( self->m_pycxx_object ) << std::dec << std::endl;
#endif
    delete self->m_pycxx_object;
    _self->ob_type->tp_free( _self );
    */
}

void PyInit_initialize()
{
    qInstallMessageHandler(myMessageOutput);
    // gce = new CtrlEngine();

    Py_Initialize();
    
    static PyObject* cModuleQt = NULL;
    // abModule.m_free = mymodule_dealloc;
    cModuleQt = PyModule_Create(&abModule);
    // cModuleQt = PyModule_New("ab");
    // m_cModuleQt = cModuleQt;

    int ok = false;
    ok = PyModule_Check(cModuleQt);

    qDebug()<<"mc.....:"<<ok<<PyModule_GetName(cModuleQt);
        //<<"'"<<PyModule_GetFilenameObject(cModuleQt)<<"'";
    qDebug()<<_Py_PackageContext;

    PyModule_AddStringConstant(cModuleQt, "hehe", "vvvvvvheheh");
    // PyType_Type();

    PyObject* dt = PyDict_New();
    // make_type("ab.QString456", (PyTypeObject*)&PyObject_Type, NULL, 0);
    // PyObject* dyc = PyObject_CallFunction((PyObject*)&PyType_Type, "sOO",
    // "QString456", &PyObject_Type, dt);
    // PyModule_AddObject(cModuleQt, "QString456", dyc);

    // see    ExtensionType.hxx:212  static PythonType &behaviors()
    //*
    auto tbl = new PyTypeObject();
    memset(tbl, 0, sizeof(PyTypeObject));
    *reinterpret_cast<PyObject*>(tbl) = py_object_initializer;
    reinterpret_cast<PyObject*>(tbl)->ob_type = &PyType_Type;
    tbl->tp_name = "ab.QString456";
    tbl->tp_basicsize = 8;
    tbl->tp_itemsize = 0;
    tbl->tp_flags |= Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    // tbl->tp_dealloc = standard_dealloc;
    tbl->tp_dealloc = extension_object_deallocator;
    //*/
    qDebug()<<"ty ready???"<<PyType_Ready(tbl); // 这个函数的调用很重要啊，相当于类型定义结束。必须要执行的。
    
    PyModule_AddObject(cModuleQt, "QString456", (PyObject*)tbl);
    PyObject_Print((PyObject*)tbl, stdout, 1);
    PyObject_Print((PyObject*)cModuleQt, stdout, 1);

    int rc = PyRun_SimpleString("XX = type('QString456', (object,), dict())");
    qDebug()<<rc<<"ffffffff";
    
    // table->tp_name = const_cast<char *>( default_name );
	// table->tp_basicsize = basic_size;
	// table->tp_itemsize = itemsize;

    // p->set_tp_new( extension_object_new );
    // p->set_tp_init( extension_object_init );
    // p->set_tp_dealloc( extension_object_deallocator );

    qDebug()<<"Hhhhhhhhh";
    /*
    // 对所有的Qt5::someconst常量的调用注册
    rb_define_module_function(cModuleQt, "const_missing", FUNVAL nx_Qt_constant_missing, -1);
    // 对所有的Qt5::somefunc()函数的调用注册
    rb_define_module_function(cModuleQt, "method_missing", FUNVAL nx_Qt_method_missing, -1);
  
    // for qApp 类Qt全局变量
    rb_define_virtual_variable("$qApp", (VALUE (*)(ANYARGS)) nx_Qt_global_variable_get,
                               (void (*)(ANYARGS)) nx_Qt_global_variable_set);
    */
}

PyObject* Hack_GetAttr(PyObject* v, PyObject* name)
{
    const char *vstr;
    PyTypeObject *tp = Py_TYPE(v);        
    if (!v) return NULL;
    vstr = tp->tp_name;
    if (strcmp(vstr, "module") == 0) {
        if (strcmp(PyModule_GetName(v), "qt5") == 0) {
            printf("inhackkkkkkkk:%s::%s\n", PyModule_GetName(v), _PyUnicode_AsString(name));
            if (strcmp(_PyUnicode_AsString(name), "abcdefg") == 0) {
                printf("inhackkkkkkkk:%s\n", PyModule_GetName(v));
            }
        }
    }
    // if (strstr(vstr, "<module 'mt'") != NULL) {
    // printf("inhackkkkkkkkk:\n");
    // }
    return NULL;
}
