
// 메모리 덩어리를 나타내는 CRTP class 이다.
// 템플릿 파라미터에 따라, 참조 가능한 버퍼가 되거나, 읽기 전용이 되거나, 할 수 있다.
//
//   KEN에 정의된 Bufferable 클래스들(C=컴포넌트 타입)
// View<C> = 읽기 전용 참조 배열
//    size: sizeof(void*) * 2
// WView<C> = 쓰기 가능 참조 배열
//    size: sizeof(void*) * 2
// Array<C> = 동적 할당 배열
//    size: sizeof(void*)
// BArray<C, N> = 정적 할당 배열 (N=컴포넌트 개수)
//    size: sizeof(C) * N + sizeof(size_t)
// Text = View<char>
// AText = Array<char>
// BText<N> = BArray<char, N>
//
// 버퍼러블은 동일한 컴포넌트 타입의 출력 스트림으로 출력시킬 수 있다.
//  ex1) cout << (Text)"test" << endl;
//  ex2) wcout << (View<wchar_t>)L"test" << endl;
// 하지만 타입이 컴포넌트 타입이 다르면 자동 변환하지는 않는다.
//  ex) wcout << "test" << endl; // 오류
//
// 읽기/쓰기 가능한 Bufferable은 출력스트림이 된다.
//  ex1) AText text; text << "test" << 'C' << endl;
//  ex2) Array<int> array; array << 1 << 2 << 3;
//
//   정의 방법
// 첫번째 템플릿 파라미터: 클래스 자기 자신
// 두번째 템플릿 파라미터: BufferInfo, Bufferable의 세부 정보를 정한다
//
//   BufferInfo 파라미터 정보
// _accessable: 직접 참조 가능 여부, 불가능 할 경우 다른곳으로 복사해와야 읽을 수 있다. 
//   true일 경우 begin(), end() 를 정의해야하며,
//   fals일 경우 size(), copyTo() 를 정의해야한다.
// _szable: 복사해올 때 NULL문자가 붙게되는 여부이다.  WINAPI의 GetWindowText 등의 함수등을 래핑할 때 사용되고 있다. _accessable 이 아닌 경우만 사용
// _readonly: 읽기 전용 여부, false일 경우 쓰기가 가능하다.
