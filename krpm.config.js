
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
		krb.vsbuild('KEN.sln', krb.config.name, krb.platform.name);
		var bin;
		if (krb.platform == js)
		{
			bin = [
				[['KR3.bc', 'KRNew.bc'], 'KR3.bc'],
				'KRApp.bc',
				'KRGame.bc'
			];
		}
		else
		{
			const libEGL = "../../../KRThird/angle/lib/"+krb.platform.name+"/"+krb.name+"/libEGL.lib";
			const libGLESv2 = "../../../KRThird/angle/lib/"+krb.platform.name+"/"+krb.name+"/libGLESv2.lib";

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
			'bin/' + krb.platform.shortName + '/' + krb.name,
			krb.libExportDir + '/' + krb.name, bin);
	}
};
