/***************************************************************************
      Interpolation.cpp  -  Interpolation types
			     -------------------
    begin                : Sat Feb 03 2001
    copyright            : (C) 2001 by Thomas Eschenbacher
    email                : Thomas Eschenbacher <thomas.eschenbacher@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "config.h"
#include "klocale.h"

#include "Curve.h"
#include "Interpolation.h"

//***************************************************************************
//***************************************************************************
void Interpolation::InterpolationMap::fill()
{
    append(INTPOL_LINEAR,      0, "linear",      "linear");
    append(INTPOL_SPLINE,      1, "spline",      "spline");
    append(INTPOL_NPOLYNOMIAL, 2, "n-polynom",   "polynom, nth degree");
    append(INTPOL_POLYNOMIAL3, 3, "3-polynom",   "polynom, 3rd degree");
    append(INTPOL_POLYNOMIAL5, 4, "5-polynom",   "polynom, 5th degree");
    append(INTPOL_POLYNOMIAL7, 5, "5-polynom",   "polynom, 7th degree");
    append(INTPOL_SAH,         6, "sample_hold", "sample and hold");

#undef NEVER_COMPILE_THIS
#ifdef NEVER_COMPILE_THIS
#error "this could produce problems in plugins and/or libs when \
        loaded before the main application is up."
    i18n("Linear");
    i18n("Spline");
    i18n("N-Polynom");
    i18n("3-Polynom");
    i18n("5-Polynom");
    i18n("7-Polynom");
    i18n("Polynom, N-th Degree");
    i18n("Polynom, 3rd Degree");
    i18n("Polynom, 5th Degree");
    i18n("Polynom, 7th Degree");
    i18n("Sample and Hold");
#endif
}

//***************************************************************************
//***************************************************************************

// static initializer
Interpolation::InterpolationMap Interpolation::m_interpolation_map;

//***************************************************************************
Interpolation::Interpolation(interpolation_t type)
    :m_curve(), m_x(), m_y(), m_der(), m_type(type)
{
}

//***************************************************************************
Interpolation::~Interpolation()
{
}

//***************************************************************************
QStringList Interpolation::descriptions(bool localized)
{
    QStringList list;
    unsigned int count = m_interpolation_map.count();
    unsigned int i;
    for (i=0; i < count; i++) {
	interpolation_t index = m_interpolation_map.findFromData(i);
	list.append(m_interpolation_map.description(index, localized));
    }
    return list;
}

//***************************************************************************
QString Interpolation::name(interpolation_t type)
{
    return m_interpolation_map.name(type);
}

//***************************************************************************
unsigned int Interpolation::count()
{
    return (m_curve ? m_curve->count() : 0);
}

//***************************************************************************
double Interpolation::singleInterpolation(double input)
{
    if (!count()) return 0.0; // no data ?

    unsigned int degree = 0;
    unsigned int count = this->count();

    if (input < 0.0) input = 0.0;
    if (input > 1.0) input = 1.0;

    switch (m_type) {
	case INTPOL_LINEAR:
	    {
		unsigned int i = 1;
		while ((m_x[i] < input) && (i < count))
		    i++;

		double dif1 = m_x[i] - m_x[i-1];  //!=0 per definition
		double dif2 = input - m_x[i-1];

		return (m_y[i-1] + ((m_y[i] - m_y[i-1])*dif2 / dif1));
	    }
	case INTPOL_SPLINE:
	    {
		double a, b, diff;
		unsigned int j = 1;

		while ((m_x[j] < input) && (j < count))
		    j++;

		diff = m_x[j] - m_x[j-1];

		a = (m_x[j] - input) / diff;    //div should not be 0
		b = (input - m_x[j-1]) / diff;

		return (a*m_y[j-1] + b*m_y[j] + ((a*a*a - a)*m_der[j - 1] +
		       (b*b*b - b)*m_der[j])*(diff*diff) / 6);
	    }
	case INTPOL_NPOLYNOMIAL:
	    {
		double ny = m_y[0];
		for (unsigned int j = 1; j < count; j++)
		    ny = ny * (input - m_x[j]) + m_y[j];
		return ny;
	    }
	case INTPOL_SAH:     //Sample and hold
	    {
		unsigned int i = 1;
		while ((m_x[i] < input) && (i < count))
		    i++;
		return m_y[i-1];
	    }
	case INTPOL_POLYNOMIAL3:
	    degree = 3;
	    break;
	case INTPOL_POLYNOMIAL5:
	    degree = 5;
	    break;
	case INTPOL_POLYNOMIAL7:
	    degree = 7;
	    break;
    }

    if (degree && (degree <= 7)) {
	Q_ASSERT(m_curve);
	if (!m_curve) return 0;

	// use polynom
	double ny;
	QVector<double> ax(7);
	QVector<double> ay(7);

	unsigned int i = 1;
	while ((m_x[i] < input) && (i < count))
	    i++;

	createPolynom(*m_curve, ax, ay, i - 1 - degree/2, degree);

	ny = ay[0];
	for (unsigned int j = 1; j < degree; j++)
	    ny = ny * (input - ax[j]) + ay[j];
	return ny;
    }

    return 0;
}

//***************************************************************************
bool Interpolation::prepareInterpolation(const Curve &points)
{
    m_curve = &points;
    if (!count()) return false; // no data ?

    m_x.resize(count()+1);
    m_y.resize(count()+1);
    m_der.resize(0);

    unsigned int c = 0;
    Curve::ConstIterator it(points);
    while (it.hasNext()) {
	Curve::Point p = it.next();
	m_x[c] = p.x();
	m_y[c] = p.y();
	c++;
    }
    m_x[c] = m_y[c] = 0.0;

    switch (m_type) {
	case INTPOL_NPOLYNOMIAL:
	    createFullPolynom(points, m_x, m_y);
	    break;
	case INTPOL_SPLINE:
	    m_der.resize(count() + 1);
	    get2Derivate(m_x, m_y, m_der, count());
	    break;
	default:
	    ;
    }
    return true;
}

//***************************************************************************
QVector<double> Interpolation::limitedInterpolation(const Curve &points,
                                                    unsigned int len)
{
    QVector<double> y = interpolation(points, len);
    for (unsigned int i = 0; i < len; i++) {
	if (y[i] > 1) y[i] = 1;
	if (y[i] < 0) y[i] = 0;
    }
    return y;
}

//***************************************************************************
QVector<double> Interpolation::interpolation(const Curve &points,
                                             unsigned int len)
{
    Q_ASSERT(len);
    if (!len) return QVector<double>();

    unsigned int degree = 0;
    QVector<double> y_out(len);
    qFill(y_out, 0.0);

    switch (m_type) {
	case INTPOL_LINEAR:
	{
	    Curve::Point p;
	    double x0, y0, x1, y1;
	    Curve::ConstIterator it(points);

	    if (it.hasNext()) {
		Curve::Point p = it.next();
		x0 = p.x();
		y0 = p.y();

		while (it.hasNext()) {
		    p = it.next();
		    x1 = p.x();
		    y1 = p.y();

		    double dy = (y1 - y0);
		    int dx  = static_cast<int>((x1 - x0) * len);
		    int min = static_cast<int>(x0 * len);

		    Q_ASSERT(x0 >= 0.0);
		    Q_ASSERT(x1 <= 1.0);
		    for (int i = static_cast<int>(x0 * len);
		         i < static_cast<int>(x1 * len); i++) {
			double h = dx ? ((double(i - min)) / dx) : 0.0;
			y_out[i] = y0 + (h * dy);
		    }
		    x0 = x1;
		    y0 = y1;
		}
	    }
	    break;
	}
	case INTPOL_SPLINE:
	{
	    int t = 1;
	    unsigned int count = points.count();

	    double ny = 0;
	    QVector<double> der(count + 1);
	    QVector<double> x(count + 1);
	    QVector<double> y(count + 1);

	    Curve::ConstIterator it(points);
	    while (it.hasNext()) {
		Curve::Point p = it.next();
		x[t] = p.x();
		y[t] = p.y();
		t++;
	    }

	    get2Derivate(x, y, der, count);

	    int ent;
	    int start = static_cast<int>(x[1] * len);

	    for (unsigned int j = 2; j <= count; j++) {
		ent = static_cast<int>(x[j] * len);
		for (int i = start; i < ent; i++) {
		    double xin = static_cast<double>(i) / len;
		    double h, b, a;

		    h = x[j] - x[j - 1];

		    if (h != 0) {
			a = (x[j] - xin) / h;
			b = (xin - x[j - 1]) / h;

			ny = (a * y[j - 1] + b * y[j] +
				((a * a * a - a) * der[j - 1] +
				(b * b * b - b) * der[j]) * (h * h) / 6.0);
		    }

		    y_out[i] = ny;
		    start = ent;
		}
	    }
	    break;
	}
	case INTPOL_POLYNOMIAL3:
	    if (!degree) degree = 3;
	case INTPOL_POLYNOMIAL5:
	    if (!degree) degree = 5;
	case INTPOL_POLYNOMIAL7:
	{
	    if (!degree) degree = 7;
	    unsigned int count = points.count();
	    double ny;
	    QVector<double> x(7);
	    QVector<double> y(7);
	    double ent, start;

	    if (count) {
		for (unsigned int px = 0; px < count - 1; px++) {
		    createPolynom (points, x, y, px - degree / 2, degree);
		    start = points[px].x();

		    if (px >= count - degree / 2 + 1)
			ent = 1;
		    else
			ent = points[px + 1].x();

		    for (int i = static_cast<int>(start * len);
			i < static_cast<int>(ent * len); i++)
		    {
			ny = y[0];
			for (unsigned int j = 1; j < degree; j++)
			    ny = ny * ((static_cast<double>(i)) / len - x[j])
				+ y[j];

			y_out[i] = ny;
		    }
		}
	    }
	    break;
	}
	case INTPOL_NPOLYNOMIAL:
	{
	    double ny;
	    int count = points.count();

	    if (count != 0) {
		QVector<double> x(count+1);
		QVector<double> y(count+1);
		double px;

		createFullPolynom(points, x, y);

		for (unsigned int i = 1; i < len; i++) {
		    px = static_cast<double>(i) / len;
		    ny = y[0];
		    for (int j = 1; j < count; j++)
			ny = ny * (px - x[j]) + y[j];

		    y_out[i] = ny;
		}
	    }
	    break;
	}
	case INTPOL_SAH:
	{
	    double x0, y0, x1, y1;

	    Curve::ConstIterator it(points);
	    if (it.hasNext()) {
		Curve::Point p = it.next();
		x0 = p.x();
		y0 = p.y();

		while (it.hasNext()) {
		    p = it.next();
		    x1 = p.x();
		    y1 = p.y();

		    for (int i = static_cast<int>(x0 * len);
		         i < static_cast<int>(x1 * len); i++)
			y_out[i] = y0;

		    x0 = x1;
		    y0 = y1;
		}
	    }
	}
    }

    return y_out;
}

//***************************************************************************
void Interpolation::createFullPolynom(const Curve &points,
	QVector<double> &x, QVector<double> &y)
{
    Q_ASSERT(!points.isEmpty());
    Q_ASSERT(m_curve);
    if (points.isEmpty()) return;
    if (!m_curve) return;

    Q_ASSERT(points.count() == m_curve->count());
    if (points.count() != m_curve->count()) return;

    unsigned int count = 0;
    Curve::ConstIterator it(points);
    while (it.hasNext()) {
	Curve::Point p = it.next();
	x[count] = p.x();
	y[count] = p.y();
	count++;
    }

    for (unsigned int k = 0; k < count; k++)
	for (unsigned int j = k; j; ) {
	    j--;
	    y[j] = (y[j] - y[j+1]) / (x[j] - x[k]);
	}
}

//***************************************************************************
void Interpolation::get2Derivate(const QVector<double> &x,
	const QVector<double> &y, QVector<double> &ab, unsigned int n)
{
    Q_ASSERT(n);
    if (!n) return;

    unsigned int i, k;
    double p, qn, sig, un;
    QVector<double> u(n);

    ab[0] = ab[1] = 0;
    u[0] = u[1] = 0;

    for (i = 2; i < n; i++) {
	sig = (x[i] - x[i-1]) / (x[i+1] - x[i-1]);
	p = sig * ab[i-1] + 2;
	ab[i] = (sig-1) / p;
	u[i] = (y[i+1] - y[i])   / (x[i+1] - x[i])
	     - (y[i]   - y[i-1]) / (x[i]   - x[i-1]);
	u[i] = (6 * u[i] / (x[i+1] - x[i-1]) - sig * u[i-1]) / p;
    }

    qn = 0;
    un = 0;
    ab[n] = (un - qn * u[n - 1]) / (qn * ab[n - 1] + 1);

    for (k = n - 1; k > 0; k--)
	ab[k] = ab[k] * ab[k + 1] + u[k];

}

//***************************************************************************
void Interpolation::createPolynom(const Curve &points,
                                  QVector<double> &x, QVector<double> &y,
                                  int pos, unsigned int degree)
{
    unsigned int count = 0;
    Curve::ConstIterator it(points);
    if (!it.hasNext()) return;
    Curve::Point p = it.next();

    if (pos < 0) {
	switch (pos) {
	   case -3:
		x[count] = -1.5;
		y[count++] = p.y();
		pos++;
	   case -2:
		x[count] = -1.0;
		y[count++] = p.y();
		pos++;
	   case -1:
		x[count] = -0.5;
		y[count++] = p.y();
		pos++;
	}
    }

    for (int i = 0; i < pos; i++)
	p = it.next();

    while ((count < degree) && it.hasNext()) {
	p = it.next();
	x[count]   = p.x();
	y[count++] = p.y();
    }

    int i = 1;
    it.toBack();
    p = it.previous();
    while (count < degree) {
	x[count]   = 1.0 + 0.5 * (i++);
	y[count++] = p.y();
    }

    // create coefficients in y[i] and x[i];
    for (unsigned int k = 0; k < degree; k++)
	for (int j = k - 1; j >= 0; j--)
	    y[j] = (y[j] - y[j + 1]) / (x[j] - x[k]);

}

//***************************************************************************
//***************************************************************************
