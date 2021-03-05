#include "pch.h"
#include "libdspl-2.0.h"


int polyval(const double *a, int ord, double *x, int n, double *y)
{
	if (!a || !x || !y)
	{
		return ERROR_PTR;
	}

	if (ord < 0)
	{
		return ERROR_POLY_ORD;
	}

	if (n < 1)
	{
		return ERROR_SIZE;
	}

	for (int k = 0; k < n; k++)
	{
		y[k] = a[ord];

		for (int m = ord - 1; m > -1; m--)
		{
			y[k] = y[k] * x[k] + a[m];
		}
	}

	return 0;
}


int farrow_lagrange(double *s, int n, double p, double q, double frd,
                    double **y, int &ny)
{
	constexpr auto DSPL_FARROW_LAGRANGE_COEFF = 1.0 / 6.0; //0.16666666666666666666666666666667
	constexpr auto AR_SIZE = 4;
	double a[AR_SIZE];
	double g[AR_SIZE];
	double *z;
	int res = 0;
	(*y) = nullptr;
	ny = 0;

	if (n < 1)
	{
		return ERROR_SIZE;
	}

	if (p <= 0.0 || q <= 0.0)
	{
		return ERROR_RESAMPLE_RATIO;
	}

	if (frd <= -1.0 || frd >= 1.0)
	{
		return ERROR_RESAMPLE_FRAC_DELAY;
	}

	double dt = q / p;
	int newlen = int(double(n - 1) / dt) + 1;
	(*y) = new double[newlen];

	if ((*y) == nullptr)
	{
		return ERROR_PTR;
	}

	double t = -frd;
	int k = 0;

	while (k < newlen)
	{
		double ft = floor(t);
		double x = t - (ft + 1.0);
		int ind = int(ft) - 1;

		if (ind < 0)
		{
			memset(g, 0, AR_SIZE * sizeof(double));
			//g[3] = g[2] = g[1] = g[0] = 0;

			if (ind > (-3))
			{
				memcpy(g - ind, s, (AR_SIZE + ind) * sizeof(double));
				//for (int i = -ind, j = 0; i < AR_SIZE; ++i, ++j)
				//{
				//  g[i] = s[j];
				//}
			}

			z = g;
		}
		else
		{
			if (ind < n - 3)
			{
				z = s + ind;
				//z = &s[ind];
			}
			else
			{
				memset(g, 0, AR_SIZE * sizeof(double));
				//g[3] = g[2] = g[1] = g[0] = 0;

				if ((n - ind) > 0)
				{
					memcpy(g, s + ind, (n - ind) * sizeof(double));
					//for (int i = ind, j = 0; i < n; ++i, ++j)
					//{
					//  g[j] = s[i];
					//}
				}

				z = g;
			}
		}

		a[0] = z[2];
		a[3] = DSPL_FARROW_LAGRANGE_COEFF * (z[3] - z[0]) + 0.5 * (z[1] - z[2]);
		a[1] = 0.5 * (z[3] - z[1]) - a[3];
		a[2] = z[3] - z[2] - a[3] - a[1];
		res = polyval(a, 3, &x, 1, (*y) + k);

		if (res != RES_OK)
		{
			break;
		}

		t += dt;
		k++;
	}

	ny = k;
	return res;
}

int farrow_spline(double *s, int n, double p, double q, double frd,
                  double **y, int &ny)
{
	constexpr auto AR_SIZE = 4;
	double a[AR_SIZE];
	double g[AR_SIZE];
	double *z;
	int res = 0;
	(*y) = nullptr;
	ny = 0;

	if (n < 1)
	{
		return ERROR_SIZE;
	}

	if (p <= 0.0 || q <= 0.0)
	{
		return ERROR_RESAMPLE_RATIO;
	}

	if (frd <= -1.0 || frd >= 1.0)
	{
		return ERROR_RESAMPLE_FRAC_DELAY;
	}

	double dt = q / p;
	int newlen = int(double(n - 1) / dt) + 1;
	(*y) = new double[newlen];

	if ((*y) == nullptr)
	{
		return ERROR_PTR;
	}

	double t = -frd;
	int k = 0;

	while (k < newlen)
	{
		double ft = floor(t);
		double x = t - (ft + 1.0);
		int ind = int(ft) - 1;

		if (ind < 0)
		{
			memset(g, 0, AR_SIZE * sizeof(double));
			//g[3] = g[2] = g[1] = g[0] = 0;

			if (ind > (-3))
			{
				memcpy(g - ind, s, (AR_SIZE + ind) * sizeof(double));
				//for (int i = -ind, j = 0; i < AR_SIZE; ++i, ++j)
				//{
				//  g[i] = s[j];
				//}
			}

			z = g;
		}
		else
		{
			if (ind < n - 3)
			{
				z = s + ind;
				//z = &s[ind];
			}
			else
			{
				memset(g, 0, AR_SIZE * sizeof(double));
				//g[3] = g[2] = g[1] = g[0] = 0;

				if ((n - ind) > 0)
				{
					memcpy(g, s + ind, (n - ind) * sizeof(double));
					//for (int i = ind, j = 0; i < n; ++i, ++j)
					//{
					//  g[j] = s[i];
					//}
				}

				z = g;
			}
		}

		a[0] = z[2];
		a[1] = 0.5 * (z[3] - z[1]);
		a[3] = 2.0 * (z[1] - z[2]) + a[1] + 0.5 * (z[2] - z[0]);
		a[2] = z[1] - z[2] + a[3] + a[1];
		res = polyval(a, 3, &x, 1, (*y) + k);

		if (res != RES_OK)
		{
			break;
		}

		t += dt;
		k++;
	}

	ny = k;
	return res;
}

int fir_linphase(int ord, double w0, double w1, FIR_FILTER filter_type,
                 FIR_WINDOW win_type, bool symmetric, double win_param, double *h)
{
	int ord1 = ord + 1;
	int err;

	if (ord < 1)
	{
		return ERROR_FILTER_ORD;
	}

	if (w0 <= 0.0)
	{
		return ERROR_FILTER_WP;
	}

	if (!h)
	{
		return ERROR_PTR;
	}

	switch (filter_type)
	{
		/* Lowpass FIR coefficients calculation */
		case FIR_FILTER::LOWPASS:
			err = fir_linphase_lpf(ord, w0, win_type, symmetric, win_param, h);
			break;

		/* Highpass FIR coefficients calculation */
		case FIR_FILTER::HIGHPASS:
			err = fir_linphase_lpf(ord, 1.0 - w0, win_type, symmetric, win_param, h);

			if (err == RES_OK)
			{
				/* LOWPASS filter frequency inversion */
				for (int n = 0; n < ord1; n += 2)
				{
					h[n] = -h[n];
				}
			}

			break;

		/* Bandpass FIR coefficients calculation */
		case FIR_FILTER::BANDPASS:
		{
			if (w1 < w0)
			{
				err = ERROR_FILTER_WS;
				break;
			}

			double wc = (w0 + w1) * 0.5; /* central frequency */
			double b = w1 - w0;        /* bandwidth */
			err = fir_linphase_lpf(ord, b * 0.5, win_type, symmetric, win_param, h);

			if (err == RES_OK)
			{
				/* LOWPASS frequency shifting to the central frequency */
				double del = 0.5 * double(ord);

				for (int n = 0; n < ord1; n++)
				{
					h[n] *= 2.0 * cos(M_PI * (double(n) - del) * wc);
				}
			}

			break;
		}

		/* BandStop FIR coefficients calculation */
		/* ATTENTION! Bandstop filter must be even order only! */
		case FIR_FILTER::BANDSTOP:
		{
			/* check filter order. Return error if order is odd. */
			if (ord % 2)
			{
				return ERROR_FILTER_ORD;
			}

			/* check frequency (w1 must be higher than w0) */
			if (w1 < w0)
			{
				err = ERROR_FILTER_WS;
				break;
			}

			/* temp coeff vector */
			auto h0 = new double[ord1];

			if (h0)
			{
				/* calculate LOWPASS */
				err = fir_linphase(ord, w0, 0.0, FIR_FILTER::LOWPASS,
				                   win_type, symmetric, win_param, h0);

				if (err == RES_OK)
				{
					/* calculate HIGHPASS */
					err = fir_linphase(ord, w1, 0.0, FIR_FILTER::HIGHPASS,
					                   win_type, symmetric, win_param, h);

					if (err == RES_OK)
					{
						/* Bandstop filter is sum of lowpass and highpass filters */
						for (int n = 0; n < ord1; n++)
						{
							h[n] += h0[n];
						}
					}
				}

				delete[] h0;
			}

			break;
		}

		default:
			err = ERROR_FILTER_FT;
	}

	return err;
}


int fir_linphase_lpf(int ord, double wp, FIR_WINDOW win_type, bool symmetric, double win_param, double *h)
{
	int err = RES_OK;
	int ord1 = ord + 1;
	auto w = new double[ord1];

	if (w)
	{
		if ((err = linspace(-0.5 * double(ord), 0.5 * double(ord), ord1, true, w)) == RES_OK)
		{
			if ((err = sinc(w, ord1, M_PI * wp, h)) == RES_OK)
			{
				if ((err = window(w, ord1, win_type, symmetric, win_param)) == RES_OK)
				{
					for (int n = 0; n < ord1; n++)
					{
						h[n] *= w[n] * wp;
					}
				}
			}
		}

		delete[] w;
	}

	return err;
}


int linspace(double x0, double x1, int n, bool symmetric, double *x)
{
	if (n < 2)
	{
		return ERROR_SIZE;
	}

	if (!x)
	{
		return ERROR_PTR;
	}

	if (symmetric)
	{
		double dx = (x1 - x0) / double(n - 1);
		x[0] = x0;

		for (int k = 1; k < n; k++)
		{
			x[k] = x[k - 1] + dx;
		}
	}
	else
	{
		double dx = (x1 - x0) / double(n);
		x[0] = x0;

		for (int k = 1; k < n; k++)
		{
			x[k] = x[k - 1] + dx;
		}
	}

	return RES_OK;
}

int sinc(double *x, int n, double a, double *y)
{
	if (!x || !y)
	{
		return ERROR_PTR;
	}

	if (n < 1)
	{
		return ERROR_SIZE;
	}

	for (int k = 0; k < n; k++)
	{
		y[k] = (x[k] == 0.0) ? 1.0 : sin(a * x[k]) / (a * x[k]);
	}

	return RES_OK;
}

int window(double *w, int n, FIR_WINDOW win_type, bool symmetric, double param)
{
	switch (win_type)
	{
		case FIR_WINDOW::BARTLETT:
			return win_bartlett(w, n, symmetric);

		case FIR_WINDOW::BARTLETT_HANNING:
			return win_bartlett_hann(w, n, symmetric);

		case FIR_WINDOW::BLACKMAN:
			return win_blackman(w, n, symmetric);

		case FIR_WINDOW::BLACKMAN_HARRIS:
			return win_blackman_harris(w, n, symmetric);

		case FIR_WINDOW::BLACKMAN_NUTTALL:
			return win_blackman_nuttall(w, n, symmetric);

		case FIR_WINDOW::CHEBY:
			return win_cheby(w, n, param);

		case FIR_WINDOW::FLAT_TOP:
			return win_flat_top(w, n, symmetric);

		case FIR_WINDOW::GAUSSIAN:
			return win_gaussian(w, n, symmetric, param);

		case FIR_WINDOW::HAMMING:
			return win_hamming(w, n, symmetric);

		case FIR_WINDOW::HANNING:
			return win_hann(w, n, symmetric);

		case FIR_WINDOW::KAISER:
			return win_kaiser(w, n, symmetric, param);

		case FIR_WINDOW::LANCZOS:
			return win_lanczos(w, n, symmetric);

		case FIR_WINDOW::NUTTALL:
			return win_nuttall(w, n, symmetric);

		case FIR_WINDOW::RECTANGULAR:
			return win_rect(w, n);

		case FIR_WINDOW::COS:
			return win_cos(w, n, symmetric);

		default:
			return ERROR_WIN_TYPE;
	}

	return RES_OK;
}

int win_bartlett(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? double(n - 1) : double(n);
	double a = x * 0.5;
	double b = 2.0 / x;

	for (int i = 0; i < n; i++)
	{
		//w[i] = 2.0 / x * (x * 0.5 - fabs(double(i) - x * 0.5));
		w[i] = b * (a - fabs(double(i) - a));
	}

	return RES_OK;
}

int win_bartlett_hann(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? 1.0 / double(n - 1) : 1.0 / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = 0.62 - 0.48 * fabs(y - 0.5) - 0.38 * cos(M_2PI * y);
		y += x;
	}

	return RES_OK;
}

int win_blackman(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = 0.42 - 0.5 * cos(y) + 0.08 * cos(2.0 * y);
		y += x;
	}

	return RES_OK;
}

int win_blackman_harris(double *w, int n, bool symmetric)
{
	constexpr double a0 = 0.35875;
	constexpr double a1 = 0.48829;
	constexpr double a2 = 0.14128;
	constexpr double a3 = 0.01168;

	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = a0 - a1 * cos(y) + a2 * cos(2.0 * y) - a3 * cos(3.0 * y);
		y += x;
	}

	return RES_OK;
}

int win_blackman_nuttall(double *w, int n, bool symmetric)
{
	constexpr double a0 = 0.3635819;
	constexpr double a1 = 0.4891775;
	constexpr double a2 = 0.1365995;
	constexpr double a3 = 0.0106411;

	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = a0 - a1 * cos(y) + a2 * cos(2.0 * y) - a3 * cos(3.0 * y);
		y += x;
	}

	return RES_OK;
}

int win_cheby(double *w, int n, double param)
{
	int m;
	double dz, chy;

	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	if (param <= 0.0)
	{
		return ERROR_WIN_PARAM;
	}

	double r1 = pow(10, param / 20);
	double x0 = cosh((1.0 / double(n - 1)) * acosh(r1));

	/* check window length even or odd */
	if (n % 2 == 0)
	{
		dz = 0.5;
		m = n / 2 - 1;
	}
	else
	{
		dz = 0.0;
		m = (n - 1) / 2;
	}

	double wmax = 0.0;

	for (int k = 0; k < m + 2; k++)
	{
		double z = double(k - m) - dz;
		double sum = 0.0;

		for (int i = 1; i <= m; i++)
		{
			double in = double(i) / double(n);
			double chx = x0 * cos(M_PI * in);
			cheby_poly1(&chx, 1, n - 1, &chy);
			sum += chy * cos(2.0 * z * M_PI * in);
		}

		w[k] = r1 + 2.0 * sum;
		w[n - 1 - k] = w[k];

		/* max value calculation */
		if (w[k] > wmax)
		{
			wmax = w[k];
		}
	}

	/* normalization */
	for (int k = 0; k < n; k++)
	{
		w[k] /= wmax;
	}

	return RES_OK;
}

int win_cos(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_PI / double(n - 1) : M_PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = sin(y);
		y += x;
	}

	return RES_OK;
}

int win_flat_top(double *w, int n, bool symmetric)
{
	constexpr double a0 = 1.0;
	constexpr double a1 = 1.93;
	constexpr double a2 = 1.29;
	constexpr double a3 = 0.388;
	constexpr double a4 = 0.032;

	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = a0 - a1 * cos(y) + a2 * cos(2.0 * y) - a3 * cos(3.0 * y) + a4 * cos(4.0 * y);
		y += x;
	}

	return RES_OK;
}

int win_gaussian(double *w, int n, bool symmetric, double alpha)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double a = symmetric ? double(n - 1) * 0.5 : double(n) * 0.5;
	double sigma = 1.0 / (alpha * a);

	for (int i = 0; i < n; i++)
	{
		double y = (double(i) - a) * sigma;
		w[i] = exp(-0.5 * y * y);
	}

	return RES_OK;
}

int win_hamming(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = 0.54 - 0.46 * cos(y);
		y += x;
	}

	return RES_OK;
}

int win_hann(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = 0.5 * (1 - cos(y));
		y += x;
	}

	return RES_OK;
}

int win_kaiser(double *w, int n, bool symmetric, double param)
{
	double num, den;

	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double L = symmetric ? double(n - 1) / 2.0 : double(n) / 2.0;
	int err = bessel_i0(&param, 1, &den);

	if (err != RES_OK)
	{
		return err;
	}

	for (int i = 0; i < n; i++)
	{
		double x = 2.0 * (double(i) - L) / double(n);
		double y = param * sqrt(1.0 - x * x);
		err = bessel_i0(&y, 1, &num);

		if (err != RES_OK)
		{
			return err;
		}

		w[i] = num / den;
	}

	return err;
}

int win_lanczos(double *w, int n, bool symmetric)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		if ((y - M_PI) == 0.0)
		{
			w[i] = 1.0;
		}
		else
		{
			w[i] = sin(y - M_PI) / (y - M_PI);
		}

		y += x;
	}

	return RES_OK;
}

int win_nuttall(double *w, int n, bool symmetric)
{
	constexpr double a0 = 0.355768;
	constexpr double a1 = 0.487396;
	constexpr double a2 = 0.144232;
	constexpr double a3 = 0.012604;

	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	double x = symmetric ? M_2PI / double(n - 1) : M_2PI / double(n);
	double y = 0.0;

	for (int i = 0; i < n; i++)
	{
		w[i] = a0 - a1 * cos(y) + a2 * cos(2.0 * y) - a3 * cos(3.0 * y);
		y += x;
	}

	return RES_OK;
}

int win_rect(double *w, int n)
{
	if (!w)
	{
		return ERROR_PTR;
	}

	if (n < 2)
	{
		return ERROR_SIZE;
	}

	for (int i = 0; i < n; i++)
	{
		w[i] = 1.0;
	}

	return RES_OK;
}


int bessel_i0(double *x, int n, double *y)
{
	const double P16[17] = { 1.0000000000000000000000801e+00,
	                         2.4999999999999999999629693e-01,
	                         2.7777777777777777805664954e-02,
	                         1.7361111111111110294015271e-03,
	                         6.9444444444444568581891535e-05,
	                         1.9290123456788994104574754e-06,
	                         3.9367598891475388547279760e-08,
	                         6.1511873265092916275099070e-10,
	                         7.5940584360755226536109511e-12,
	                         7.5940582595094190098755663e-14,
	                         6.2760839879536225394314453e-16,
	                         4.3583591008893599099577755e-18,
	                         2.5791926805873898803749321e-20,
	                         1.3141332422663039834197910e-22,
	                         5.9203280572170548134753422e-25,
	                         2.0732014503197852176921968e-27,
	                         1.1497640034400735733456400e-29
	                       };
	const double P22[23] = { 3.9894228040143265335649948e-01,
	                         4.9867785050353992900698488e-02,
	                         2.8050628884163787533196746e-02,
	                         2.9219501690198775910219311e-02,
	                         4.4718622769244715693031735e-02,
	                         9.4085204199017869159183831e-02,
	                         -1.0699095472110916094973951e-01,
	                         2.2725199603010833194037016e+01,
	                         -1.0026890180180668595066918e+03,
	                         3.1275740782277570164423916e+04,
	                         -5.9355022509673600842060002e+05,
	                         2.6092888649549172879282592e+06,
	                         2.3518420447411254516178388e+08,
	                         -8.9270060370015930749184222e+09,
	                         1.8592340458074104721496236e+11,
	                         -2.6632742974569782078420204e+12,
	                         2.7752144774934763122129261e+13,
	                         -2.1323049786724612220362154e+14,
	                         1.1989242681178569338129044e+15,
	                         -4.8049082153027457378879746e+15,
	                         1.3012646806421079076251950e+16,
	                         -2.1363029690365351606041265e+16,
	                         1.6069467093441596329340754e+16
	                       };

	if (!x || !y)
	{
		return ERROR_PTR;
	}

	if (n < 1)
	{
		return ERROR_SIZE;
	}

	for (int k = 0; k < n; k++)
	{
		if (x[k] < 0.0)
		{
			return ERROR_NEGATIVE;
		}

		if (x[k] < 7.75)
		{
			double x2 = x[k] * x[k] * 0.25;
			polyval(P16, 16, &x2, 1, y + k);
			y[k] = x2 * y[k] + 1.0;
		}
		else
		{
			double x2 = 1.0 / x[k];
			polyval(P22, 22, &x2, 1, y + k);
			y[k] *= exp(x[k]) / sqrt(x[k]);
		}
	}

	return RES_OK;
}

int cheby_poly1(double *x, int n, int ord, double *y)
{
	double t[2] = { 0.0, 0.0 };

	if (!x || !y)
	{
		return ERROR_PTR;
	}

	if (n < 1)
	{
		return ERROR_SIZE;
	}

	if (ord < 0)
	{
		return ERROR_POLY_ORD;
	}

	if (ord == 0)
	{
		for (int k = 0; k < n; k++)
		{
			y[k] = 1.0;
		}

		return RES_OK;
	}

	if (ord == 1)
	{
		memcpy(y, x, n * sizeof(double));
		return RES_OK;
	}

	for (int k = 0; k < n; k++)
	{
		int m = 2;
		t[1] = x[k];
		t[0] = 1.0;

		while (m <= ord)
		{
			y[k] = 2.0 * x[k] * t[1] - t[0];
			t[0] = t[1];
			t[1] = y[k];
			m++;
		}
	}

	return RES_OK;
}


