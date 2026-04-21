#pragma once

#include <QString>
#include "database.h"
#include "exceptions.h"

class ScheduleService {
public:
    static ScheduleService& instance() {
        static ScheduleService inst;
        return inst;
    }

    bool add(const QString& symbol, const QString& type, int qty, double targetPrice) {
        return DatabaseManager::instance().addSchedule(symbol, type, qty, targetPrice);
    }

    QVector<ScheduleRow> get(const QString& symbol = "") {
        return DatabaseManager::instance().getSchedules(symbol);
    }

    bool cancel(int id) {
        return DatabaseManager::instance().cancelSchedule(id);
    }

    bool modify(int id, int qty, double targetPrice) {
        return DatabaseManager::instance().modifySchedule(id, qty, targetPrice);
    }

private:
    ScheduleService() = default;
};
