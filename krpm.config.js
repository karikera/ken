
module.exports = {
	platforms: ['x86', 'x64', 'js'],
	static: true,
	files: [
		'KR.props',
		['KR3/kr3.natvis', 'kr3.natvis'],
		['.', '.', ['batch/*', '**/*.h', '**/*.inl', '!tester/**/*', '!KRThird/**/*', '!packages']],
		['KRThird/angle-bin', 'KRThird/angle-bin', ['**/*']]
	],
	each(krb)
	{
		krb.vsbuild('KEN.sln');
		var bin;
		if (krb.platform.shortName === 'js')
		{
			bin = [
				[['KR3.bc', 'KRNew.bc'], 'KR3.bc'],
				'KRApp.bc',
				'KRGame.bc'
			];
		}
		else
		{
			const libEGL = `../../../KRThird/angle/lib/${krb.platform.name}/${krb.config.name}/libEGL.lib`;
			const libGLESv2 = `../../../KRThird/angle/lib/${krb.platform.name}/${krb.config.name}/libGLESv2.lib`;

			bin = [
				[['KR3.lib', 'KRNew.lib'], 'KR3.lib'],
				[['KRApp.lib', libEGL, libGLESv2], 'KRApp.lib'],
				'KRSound.lib',
				'KRGame.lib',
				'KRMySQL.lib',
				'KRWin.lib',
			];
		}
		krb.copy(
			`bin/${krb.platform.shortName}/${krb.config.name}`,
			`${krb.libExportDir}/${krb.config.name}`, bin);
	}
};
