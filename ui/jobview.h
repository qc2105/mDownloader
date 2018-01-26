#ifndef JOBVIEW_H
#define JOBVIEW_H

#include <QTreeWidget>

class JobView : public QTreeWidget
{
    Q_OBJECT
public:
    JobView(QWidget * parent = 0);
    ~JobView();
};

#endif // JOBVIEW_H
