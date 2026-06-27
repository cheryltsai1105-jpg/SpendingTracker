#pragma once
#include <QString>
#include <QColor>
#include <QVector>
#include <QSettings>
#include <QDateTime>

// ─── Per-transaction entry ────────────────────────────────────────────────────
struct SpendingEntry {
    QString  title;
    double   amount;
    QColor   color;
    QString  dateTime;   // ISO-8601
};

// ─── Active spending goal ─────────────────────────────────────────────────────
struct GoalData {
    bool    active       = false;
    double  totalGoal    = 0.0;   // e.g. 500
    int     rangeCount   = 1;     // e.g. 1
    QString rangeUnit;            // "day" | "week" | "month" | "year"
    QString startDateTime;        // ISO-8601 when goal was set
    QString endDateTime;          // ISO-8601 deadline (computed on set)
};

// ─── Singleton persistent store ──────────────────────────────────────────────
class DataStore
{
public:
    static DataStore &instance() {
        static DataStore s;
        return s;
    }

    QVector<SpendingEntry> entries;
    GoalData               goal;

    // ── Helpers ──────────────────────────────────────────────────────────────

    // Sum of all entries whose dateTime falls within [start, end]
    double spentInRange(const QDateTime &start, const QDateTime &end) const {
        double sum = 0;
        for (const auto &e : entries) {
            QDateTime dt = QDateTime::fromString(e.dateTime, Qt::ISODate);
            if (dt >= start && dt <= end)
                sum += e.amount;
        }
        return sum;
    }

    // Sum for a single calendar date
    double spentOnDate(const QDate &d) const {
        double sum = 0;
        for (const auto &e : entries) {
            QDateTime dt = QDateTime::fromString(e.dateTime, Qt::ISODate);
            if (dt.date() == d) sum += e.amount;
        }
        return sum;
    }

    // ── Persistence ──────────────────────────────────────────────────────────
    void load() {
        QSettings s("SpendingTrackerApp", "SpendingTracker");

        // entries
        int n = s.beginReadArray("entries");
        entries.clear();
        for (int i = 0; i < n; ++i) {
            s.setArrayIndex(i);
            SpendingEntry e;
            e.title    = s.value("title").toString();
            e.amount   = s.value("amount").toDouble();
            e.color    = QColor(s.value("color").toString());
            e.dateTime = s.value("dateTime").toString();
            entries.append(e);
        }
        s.endArray();

        // goal
        goal.active        = s.value("goal/active",        false).toBool();
        goal.totalGoal     = s.value("goal/totalGoal",     0.0).toDouble();
        goal.rangeCount    = s.value("goal/rangeCount",    1).toInt();
        goal.rangeUnit     = s.value("goal/rangeUnit",     "week").toString();
        goal.startDateTime = s.value("goal/startDateTime", "").toString();
        goal.endDateTime   = s.value("goal/endDateTime",   "").toString();
    }

    void save() const {
        QSettings s("SpendingTrackerApp", "SpendingTracker");

        s.beginWriteArray("entries", entries.size());
        for (int i = 0; i < entries.size(); ++i) {
            s.setArrayIndex(i);
            s.setValue("title",    entries[i].title);
            s.setValue("amount",   entries[i].amount);
            s.setValue("color",    entries[i].color.name());
            s.setValue("dateTime", entries[i].dateTime);
        }
        s.endArray();

        s.setValue("goal/active",        goal.active);
        s.setValue("goal/totalGoal",     goal.totalGoal);
        s.setValue("goal/rangeCount",    goal.rangeCount);
        s.setValue("goal/rangeUnit",     goal.rangeUnit);
        s.setValue("goal/startDateTime", goal.startDateTime);
        s.setValue("goal/endDateTime",   goal.endDateTime);
    }

private:
    DataStore() { load(); }
};
