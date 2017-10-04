var field: Phi
var real: Lambda, Phi0

// input real n
// let p = phi

equation Phi {
    lap(Phi) = pow(1 - Lambda * (Phi-Phi0), n)
    bc {
        [center]    d(Phi, r) = 0
        [surface]   d(Phi, r) + Phi = 0
    }
}

equation Phi0 {
    Phi0 = Phi[0]
}

equation Lambda {
    Lambda*(Phi[1]-Phi0) = 1
}
