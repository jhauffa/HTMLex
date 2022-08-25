package data;

import math.Matrix;
import math.Vector;

public class DataRecord {

	private Matrix data;
	
	public DataRecord(Matrix data) {
		this.data = data;
	}

	public void normalize() {
		for (int i = 0; i < data.height(); i++) {
			DataSet s = new DataSet(data, i);
			s.normalize();
		}
	}
	
	public void expand(Vector v) {
		if (v.size() != data.height())
			throw new RuntimeException("size mismatch");
		for (int i = 0; i < data.height(); i++) {
			DataSet s = new DataSet(data, i);
			double min = s.min();
			double range = s.max() - min;
			v.set(i, (v.get(i) * range) + min);
		}
	}

	public void sort(IVectorCompare cmp, int permutation[]) {
		if (permutation != null) {
			if (permutation.length != data.width())
				throw new RuntimeException("invalid length of permut. array");
			for (int i = 0; i < permutation.length; i++)
				permutation[i] = i;
		}
		sort(cmp, 0, data.width() - 1, permutation);
	}

	private void swap(int a[], int i1, int i2) {
		if (a != null) {
			int tmp = a[i1];
			a[i1] = a[i2];
			a[i2] = tmp;
		}
	}
	
	private void sort(IVectorCompare cmp, int first, int last,
			int permutation[]) {
		if (first < last) {
			int mid = first - 1;
			Vector x = data.getColumn(last);
			for (int i = first; i < last; i++) {
				if (cmp.compare(data.getColumn(i), x) <= 0) {
					data.swapColumns(++mid, i);
					swap(permutation, mid, i);
				}
			}
			data.swapColumns(++mid, last);
			swap(permutation, mid, last);
			
			sort(cmp, first, mid - 1, permutation);
			sort(cmp, mid + 1, last, permutation);
		}
	}

	public void permutate(int permutation[]) {
		for (int i = 0; i < permutation.length; i++) {
			if (permutation[i] != i)
				data.swapColumns(i, permutation[i]);
		}
	}
	
	public double covariance(int y1, int y2) {
		DataSet row = new DataSet(data, y1);
		double meanY1 = row.mean();
		row = new DataSet(data, y2);
		double meanY2 = row.mean();

		int n = data.width();
		double sum = 0.0;
		for (int i = 0; i < n; i++)
			sum += (data.get(y1, i) - meanY1) * (data.get(y2, i) - meanY2);

		return (sum / (n - 1));
	}
	
	public Matrix covarianceMatrix() {
		int n = data.height();
		Matrix m = new Matrix(n, n);

		for (int i = 0; i < n; i++) {
			DataSet dataSet = new DataSet(data, i);
			m.set(i, i, dataSet.variance());
		}

		for (int i = 0; i < (n - 1); i++) {
			for (int j = i + 1; j < n; j++) {
				double covar = covariance(i, j);
				m.set(i, j, covar);
				m.set(j, i, covar);
			}
		}
		
		return m;
	}
	
}
