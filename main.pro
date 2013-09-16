CONFIG -= qt

LIBS += -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lGL -lGLU

SOURCES += main.cpp
HEADERS += 
OTHER_FILES += \ 
    shaders/vertex.glsl \
    shaders/fragment.glsl
