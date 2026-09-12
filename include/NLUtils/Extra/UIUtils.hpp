#pragma once
template<typename clst>
class DragAndDrop {
public:
    clst dataSlot;
    bool Drag(clst* source) {
        if (active) return false;
        sourceObject = source;
        active = true;
        return true;
    }
    bool IsDragged() {
        return active;
    }
    clst* GetSource() {
        return sourceObject;
    }
    bool Drop() {
        active = false;
        if (sourceObject) {
            sourceObject = nullptr;
            return true;
        }
        return false;
    }
private:
    bool active = false;
    clst* sourceObject = nullptr;
};