
// �޸� ����� ��Ÿ���� CRTP class �̴�.
// ���ø� �Ķ���Ϳ� ����, ���� ������ ���۰� �ǰų�, �б� ������ �ǰų�, �� �� �ִ�.
//
//   KEN�� ���ǵ� Bufferable Ŭ������(C=������Ʈ Ÿ��)
// View<C> = �б� ���� ���� �迭
//    size: sizeof(void*) * 2
// WView<C> = ���� ���� ���� �迭
//    size: sizeof(void*) * 2
// Array<C> = ���� �Ҵ� �迭
//    size: sizeof(void*)
// BArray<C, N> = ���� �Ҵ� �迭 (N=������Ʈ ����)
//    size: sizeof(C) * N + sizeof(size_t)
// Text = View<char>
// AText = Array<char>
// BText<N> = BArray<char, N>
//
// ���۷����� ������ ������Ʈ Ÿ���� ��� ��Ʈ������ ��½�ų �� �ִ�.
//  ex1) cout << (Text)"test" << endl;
//  ex2) wcout << (View<wchar_t>)L"test" << endl;
// ������ Ÿ���� ������Ʈ Ÿ���� �ٸ��� �ڵ� ��ȯ������ �ʴ´�.
//  ex) wcout << "test" << endl; // ����
//
// �б�/���� ������ Bufferable�� ��½�Ʈ���� �ȴ�.
//  ex1) AText text; text << "test" << 'C' << endl;
//  ex2) Array<int> array; array << 1 << 2 << 3;
//
//   ���� ���
// ù��° ���ø� �Ķ����: Ŭ���� �ڱ� �ڽ�
// �ι�° ���ø� �Ķ����: BufferInfo, Bufferable�� ���� ������ ���Ѵ�
//
//   BufferInfo �Ķ���� ����
// _accessable: ���� ���� ���� ����, �Ұ��� �� ��� �ٸ������� �����ؿ;� ���� �� �ִ�. 
//   true�� ��� begin(), end() �� �����ؾ��ϸ�,
//   fals�� ��� size(), copyTo() �� �����ؾ��Ѵ�.
// _szable: �����ؿ� �� NULL���ڰ� �ٰԵǴ� �����̴�.  WINAPI�� GetWindowText ���� �Լ����� ������ �� ���ǰ� �ִ�. _accessable �� �ƴ� ��츸 ���
// _readonly: �б� ���� ����, false�� ��� ���Ⱑ �����ϴ�.
