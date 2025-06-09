QT = core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        Base/BaseArray.cpp \
        Base/Exp/Expression.cpp \
        Base/Exp/VirtualFields.cpp \
        Base/Json/json_reader.cpp \
        Base/Json/json_value.cpp \
        Base/Json/json_writer.cpp \
        Base/LicenseEngine.cpp \
        Base/Sqlite/shell.c \
        Base/Sqlite/sqlite3.c \
        Base/StringHelper.cpp \
        Base/Variant.cpp \
        Base/bufferio.cpp \
        Base/bzib2/blocksort.c \
        Base/bzib2/bzip2.c \
        Base/bzib2/bzlib.c \
        Base/bzib2/compress.c \
        Base/bzib2/crctable.c \
        Base/bzib2/decompress.c \
        Base/bzib2/huffman.c \
        Base/bzib2/randtable.c \
        Base/classfactory.cpp \
        Base/databaseconnection.cpp \
        Base/datacoder.cpp \
        Base/datetime.cpp \
        Base/eventhelper.cpp \
        Base/file.cpp \
        Base/filepath.cpp \
        Base/guid.cpp \
        Base/http.cpp \
        Base/http/http_worker.cpp \
        Base/jsonio.cpp \
        Base/linearmath.cpp \
        Base/semaphora.cpp \
        Base/stringinterpreter.cpp \
        Base/threadpool.cpp \
        Base/version.cpp \
        Base/xml/tinystr.cpp \
        Base/xml/tinyxml.cpp \
        Base/xml/tinyxmlerror.cpp \
        Base/xml/tinyxmlparser.cpp \
        Base/xmldoc.cpp \
        Base/ziputil.cpp \
        Base/zlib/adler32.c \
        Base/zlib/compress.c \
        Base/zlib/crc32.c \
        Base/zlib/deflate.c \
        Base/zlib/gzclose.c \
        Base/zlib/gzlib.c \
        Base/zlib/gzread.c \
        Base/zlib/gzwrite.c \
        Base/zlib/infback.c \
        Base/zlib/inffast.c \
        Base/zlib/inflate.c \
        Base/zlib/inftrees.c \
        Base/zlib/ioapi.c \
        Base/zlib/trees.c \
        Base/zlib/uncompr.c \
        Base/zlib/unzip.c \
        Base/zlib/zip.c \
        Base/zlib/zutil.c \
        Datasources/colortable.cpp \
        Datasources/dataset.cpp \
        Datasources/display.cpp \
        Datasources/featureclasseditor.cpp \
        Datasources/featureclasstool.cpp \
        Datasources/featurecursor.cpp \
        Datasources/filerasterdataset.cpp \
        Datasources/histogram.cpp \
        Datasources/mapprojection.cpp \
        Datasources/mapprovider.cpp \
        Datasources/memdataarray.cpp \
        Datasources/memfeatureclass.cpp \
        Datasources/memrasterdataset.cpp \
        Datasources/queryfilter.cpp \
        Datasources/rasterbandtool.cpp \
        Datasources/selectionset.cpp \
        Datasources/shapfilefeatureclass.cpp \
        Datasources/spatialindexer.cpp \
        Datasources/statishistory.cpp \
        Datasources/streamrasterdataset.cpp \
        Datasources/tabledesc.cpp \
        Datasources/vectorprovider.cpp \
        Geometry/GDAL/geometryfactory.cpp \
        Geometry/Point.cpp \
        Geometry/ellipse.cpp \
        Geometry/envelope.cpp \
        Geometry/geometry.cpp \
        Geometry/geometrybordercapture.cpp \
        Geometry/geometrycollection.cpp \
        Geometry/geometryoperator.cpp \
        Geometry/geometrytopology.cpp \
        Geometry/labelcomputer.cpp \
        Geometry/multipolygon.cpp \
        Geometry/overlaptilegeometrygrid.cpp \
        Geometry/points.cpp \
        Geometry/polygon.cpp \
        Geometry/polyline.cpp \
        Geometry/segment.cpp \
        Geometry/segmentindexes.cpp \
        Geometry/spatialreference.cpp \
        Geometry/spline.cpp \
        Geometry/tilegeometrygrid.cpp \
        Geometry/topobuilder.cpp \
        SpatialAnalyst/featureclasstarget.cpp \
        SpatialAnalyst/featuretoraster.cpp \
        SpatialAnalyst/hydroanalyst.cpp \
        SpatialAnalyst/imagelump.cpp \
        SpatialAnalyst/rastertarget.cpp \
        SpatialAnalyst/rastertofeature.cpp \
        SpatialAnalyst/samodel.cpp \
        main.cpp \
        mcallback.cpp \
        task.cpp

win32{
    win32:QMAKE_CXXFLAGS += /bigobj
    THIRDPARTYPATH=$$PWD/../ThirdParty/win
    LIBS += -L$$THIRDPARTYPATH/GDAL/lib/ -lgdal_i
    CONFIG(release,debug|release){
        LIBS += -L$$THIRDPARTYPATH/boost/lib -llibboost_date_time-vc142-mt-x64-1_83
        LIBS += -L$$THIRDPARTYPATH/boost/lib -llibboost_thread-vc142-mt-x64-1_83
        LIBS += -L$$THIRDPARTYPATH/boost/lib -llibboost_chrono-vc142-mt-x64-1_83
    }
    else{
        LIBS += -L$$THIRDPARTYPATH/boost/lib -llibboost_date_time-vc142-mt-gd-x64-1_83
        LIBS += -L$$THIRDPARTYPATH/boost/lib -llibboost_thread-vc142-mt-gd-x64-1_83
        LIBS += -L$$THIRDPARTYPATH/boost/lib -llibboost_chrono-vc142-mt-gd-x64-1_83
    }
    INCLUDEPATH += $$THIRDPARTYPATH/GDAL/include
    DEPENDPATH += $$THIRDPARTYPATH/GDAL/include
    INCLUDEPATH += $$THIRDPARTYPATH/boost
    DEPENDPATH += $$THIRDPARTYPATH/boost
    INCLUDEPATH += $$THIRDPARTYPATH/openssl
    DEPENDPATH += $$THIRDPARTYPATH/openssl
    LIBS += -L$$THIRDPARTYPATH/openssl/lib -llibssl
    LIBS += -L$$THIRDPARTYPATH/openssl/lib -llibcrypto
}
unix{
CONFIG += plugin
    DEFINES +=BOOST_USE_LIB
    THIRDPARTYPATH=$$PWD/../ThirdParty/Linux
    LIBS += -L$$THIRDPARTYPATH/GDAL/lib/ -lgdal
    LIBS += -L$$THIRDPARTYPATH/boost/lib/ -lboost_thread
    LIBS += -L$$THIRDPARTYPATH/boost/lib/ -lboost_chrono
    LIBS += -L$$THIRDPARTYPATH/boost/lib/ -lboost_date_time
    LIBS += -L$$THIRDPARTYPATH/boost/lib/ -lboost_context
    LIBS += -L$$THIRDPARTYPATH/boost/lib/ -lboost_coroutine
    INCLUDEPATH += $$PWD/../ThirdPart/Linux/GDAL/include
    DEPENDPATH += $$THIRDPARTYPATH/GDAL/include
    INCLUDEPATH += $$THIRDPARTYPATH/boost
    DEPENDPATH += $$THIRDPARTYPATH/boost
}

HEADERS += \
    Base/BaseArray.h \
    Base/CallBack.h \
    Base/Exp/Expression.h \
    Base/Exp/ExpressionDataType.h \
    Base/Exp/VirtualFields.h \
    Base/FilePath.h \
    Base/Json/allocator.h \
    Base/Json/assertions.h \
    Base/Json/config.h \
    Base/Json/forwards.h \
    Base/Json/json.h \
    Base/Json/json_features.h \
    Base/Json/json_tool.h \
    Base/Json/json_valueiterator.inl \
    Base/Json/reader.h \
    Base/Json/value.h \
    Base/Json/version.h \
    Base/Json/writer.h \
    Base/LicenseEngine.h \
    Base/Sqlite/sqlite3.h \
    Base/Sqlite/sqlite3ext.h \
    Base/StringHelper.h \
    Base/autoptr.h \
    Base/base_globle.h \
    Base/bufferio.h \
    Base/bzib2/bzlib.h \
    Base/bzib2/bzlib_private.h \
    Base/classfactory.h \
    Base/databaseconnection.h \
    Base/datacoder.h \
    Base/datetime.h \
    Base/eventhelper.h \
    Base/file.h \
    Base/guid.h \
    Base/http.h \
    Base/http/http_worker.h \
    Base/jsonio.h \
    Base/linearmath.h \
    Base/semaphora.h \
    Base/stringinterpreter.h \
    Base/threadpool.h \
    Base/variant.h \
    Base/version.h \
    Base/xml/tinystr.h \
    Base/xml/tinyxml.h \
    Base/xmldoc.h \
    Base/ziputil.h \
    Base/zlib/crc32.h \
    Base/zlib/crypt.h \
    Base/zlib/deflate.h \
    Base/zlib/gzguts.h \
    Base/zlib/inffast.h \
    Base/zlib/inffixed.h \
    Base/zlib/inflate.h \
    Base/zlib/inftrees.h \
    Base/zlib/ioapi.h \
    Base/zlib/trees.h \
    Base/zlib/unzip.h \
    Base/zlib/zconf.h \
    Base/zlib/zip.h \
    Base/zlib/zlib.h \
    Base/zlib/zutil.h \
    Datasources/colortable.h \
    Datasources/dataset.h \
    Datasources/datasources.h \
    Datasources/display.h \
    Datasources/featureclasseditor.h \
    Datasources/featureclasstool.h \
    Datasources/featurecursor.h \
    Datasources/filerasterdataset.h \
    Datasources/histogram.h \
    Datasources/mapprojection.h \
    Datasources/mapprovider.h \
    Datasources/memdataarray.h \
    Datasources/memfeatureclass.h \
    Datasources/memrasterdataset.h \
    Datasources/queryfilter.h \
    Datasources/rasterbandtool.h \
    Datasources/selectionset.h \
    Datasources/shapfilefeatureclass.h \
    Datasources/spatialindexer.h \
    Datasources/statishistory.h \
    Datasources/streamrasterdataset.h \
    Datasources/tabledesc.h \
    Datasources/vectorprovider.h \
    Geometry/GDAL/geometryfactory.h \
    Geometry/Point.h \
    Geometry/ellipse.h \
    Geometry/envelope.h \
    Geometry/geometry.h \
    Geometry/geometry2d.h \
    Geometry/geometry3d.h \
    Geometry/geometrybase.h \
    Geometry/geometrybordercapture.h \
    Geometry/geometrycollection.h \
    Geometry/geometryoperator.h \
    Geometry/geometrytopology.h \
    Geometry/labelcomputer.h \
    Geometry/multipolygon.h \
    Geometry/overlaptilegeometrygrid.h \
    Geometry/points.h \
    Geometry/polygon.h \
    Geometry/polyline.h \
    Geometry/segment.h \
    Geometry/segmentindexes.h \
    Geometry/spatialreference.h \
    Geometry/spline.h \
    Geometry/tilegeometrygrid.h \
    Geometry/topobuilder.h \
    SpatialAnalyst/featureclasstarget.h \
    SpatialAnalyst/featuretoraster.h \
    SpatialAnalyst/hydroanalyst.h \
    SpatialAnalyst/imagelump.h \
    SpatialAnalyst/rastertarget.h \
    SpatialAnalyst/rastertofeature.h \
    SpatialAnalyst/samodel.h \
    mcallback.h \
    task.h
